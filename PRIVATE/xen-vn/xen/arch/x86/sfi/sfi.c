/*
 * sfi.c - x86 architecture SFI support.
 *
 * Copyright (c) 2009, Intel Corporation.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU General Public License,
 * version 2, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin St - Fifth Floor, Boston, MA 02110-1301 USA.
 *
 */

#include <xen/config.h>
#include <xen/types.h>
#include <asm/cache.h>
#include <asm/mm.h>
#include <asm/acpi.h>
#include <xen/init.h>
#include <asm/apic.h>
#include <asm/mpspec.h>

#include "sfi.h"

#ifdef CONFIG_X86_LOCAL_APIC
static unsigned long sfi_lapic_addr __initdata = APIC_DEFAULT_PHYS_BASE;

/* All CPUs enumerated by SFI must be present and enabled */
static void __cpuinit mp_sfi_register_lapic(u8 id)
{
	struct mpc_config_processor cpu = {
		.mpc_type = MP_PROCESSOR,
		.mpc_apicid = id,
		.mpc_apicver = GET_APIC_VERSION(apic_read(APIC_LVR)),
		.mpc_cpuflag = CPU_ENABLED,
		.mpc_cpufeature = (boot_cpu_data.x86 << 8) |
				  (boot_cpu_data.x86_model << 4) |
				  boot_cpu_data.x86_mask,
		.mpc_featureflag = boot_cpu_data.x86_capability[0]
	};

	if (MAX_LOCAL_APIC - id <= 0) {
		printk(KERN_WARNING "Processor #%d invalid (max %d)\n",
			id, MAX_LOCAL_APIC);
		return;
	}

	printk(KERN_INFO "registering lapic[%d]\n", id);
	if (id == 0)
		cpu.mpc_cpuflag |= CPU_BOOTPROCESSOR;

	MP_processor_info(&cpu);
}

static int __init sfi_parse_cpus(struct sfi_table_header *table)
{
	struct sfi_table_simple *sb;
	struct sfi_cpu_table_entry *pentry;
	int i;
	int cpu_num;

	sb = (struct sfi_table_simple *)table;
	cpu_num = SFI_GET_NUM_ENTRIES(sb, struct sfi_cpu_table_entry);
	pentry = (struct sfi_cpu_table_entry *)sb->pentry;

	for (i = 0; i < cpu_num; i++) {
		mp_sfi_register_lapic(pentry->apic_id);
		pentry++;
	}

	smp_found_config = 1;
	return 0;
}
#endif /* CONFIG_X86_LOCAL_APIC */

#ifdef CONFIG_X86_IO_APIC

static int __init sfi_parse_ioapic(struct sfi_table_header *table)
{
	struct sfi_table_simple *sb;
	struct sfi_apic_table_entry *pentry;
	int i, num;

	sb = (struct sfi_table_simple *)table;
	num = SFI_GET_NUM_ENTRIES(sb, struct sfi_apic_table_entry);
	pentry = (struct sfi_apic_table_entry *)sb->pentry;

	for (i = 0; i < num; i++) {
		mp_register_ioapic(i, pentry->phys_addr, mp_gsi_top);
		pentry++;
	}

	if (pic_mode)
	  printk(KERN_WARNING
		"SFI: pic_mod shouldn't be 1 when IOAPIC table is present\n");
	pic_mode = 0;
	return 0;
}
#endif /* CONFIG_X86_IO_APIC */

/*
 * sfi_platform_init(): register lapics & io-apics
 */
int __init sfi_platform_init(void)
{
#ifdef CONFIG_X86_LOCAL_APIC
	mp_register_lapic_address(sfi_lapic_addr);
	sfi_table_parse(SFI_SIG_CPUS, NULL, NULL, sfi_parse_cpus);
#endif
#ifdef CONFIG_X86_IO_APIC
	sfi_table_parse(SFI_SIG_APIC, NULL, NULL, sfi_parse_ioapic);
#endif
	return 0;
}
