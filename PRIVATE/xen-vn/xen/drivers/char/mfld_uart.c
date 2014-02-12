/*
 * xen/drivers/char/mfld_uart.c
 *
 * Driver for Medfield SPI UART.
 *
 * Octavian Purdila <octavian.purdila@intel.com>
 *
 * Copyright (c) 2012 Intel Corp.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include <xen/config.h>
#include <xen/console.h>
#include <xen/serial.h>
#include <xen/init.h>
#include <xen/irq.h>

#include <asm/io.h>
#include <asm/fixmap.h>
#include <asm/page.h>


#define UART_TX                 0x00
#define UART_FCR                0x02
#define UART_FCR_ENABLE_FIFO    (1 << 0)
#define UART_LSR                0x05
#define UART_LSR_TEMT           (1 << 6)
#define UART_LSR_TDRQ           (1 << 5)

#define HSU_BASE_ADDR		0xffa28000
#define HSU_PORT0_BASE		0x80
#define HSU_PORT1_BASE		0x100
#define HSU_PORT2_BASE		0x180

#define HSU_TEMP_VA		0xc0028000

#define PMU_BASE_ADDR		0xff11d000
#define UART_OFFSET		0x38

static u8 *hsu_addr;
static u32 *pmu_addr;
extern bool_t pmu_status_check;

#define serial_permitted	(!pmu_status_check || !(*pmu_addr & (0x3 << 18)))

static void __init mfld_uart_init_preirq(struct serial_port *port)
{
    l2_pgentry_t *pl2e;
    u32 l2page_addr = HSU_BASE_ADDR & ~((1<<L2_PAGETABLE_SHIFT)-1);

    /* The Medfield serial UART is located at a high physical address
       and we need to map it in the virtual space but at this point
       the paging setup is not complete. Map a temporary 2MB page
       somewhere below the Xen reserved virtual address space. */
    hsu_addr = (u8*)HSU_TEMP_VA + HSU_PORT2_BASE;

    pl2e = &idle_pg_table_l2[l2_linear_offset(HSU_TEMP_VA)];
    l2e_write(pl2e, l2e_from_paddr(l2page_addr, __PAGE_HYPERVISOR_NOCACHE | _PAGE_PAT));
}

static void __init mfld_uart_init_postirq(struct serial_port *port)
{
    l2_pgentry_t *pl2e;

    /* unmap the temporary page */
    pl2e = &idle_pg_table_l2[l2_linear_offset(HSU_TEMP_VA)];
    l2e_write(pl2e, l2e_from_paddr(0, 0));

    /* now that the paging is initialized we can properly map it */
    set_fixmap_nocache(FIX_MFLD_UART, HSU_BASE_ADDR);
    hsu_addr =  (u8*)fix_to_virt(FIX_MFLD_UART) + HSU_PORT2_BASE;

    set_fixmap_nocache(FIX_MFLD_PMU, PMU_BASE_ADDR);
    pmu_addr = (u32 *)((u8 *)fix_to_virt(FIX_MFLD_PMU) + UART_OFFSET);
}

static int mfld_uart_tx_empty(struct serial_port *port)
{
    u8 status = UART_LSR_TEMT;

    if (serial_permitted)
        status = readb(hsu_addr + UART_LSR);

    return !!(status & (UART_LSR_TEMT | UART_LSR_TDRQ));
}

static void mfld_uart_putc(struct serial_port *port, char c)
{
    if (serial_permitted)
        writeb(c, hsu_addr + UART_TX);
}


static struct uart_driver __read_mostly mfld_uart_driver = {
    .init_preirq  = mfld_uart_init_preirq,
    .init_postirq = mfld_uart_init_postirq,
    .endboot      = NULL,
    .tx_empty     = mfld_uart_tx_empty,
    .putc         = mfld_uart_putc,
    .getc         = NULL,
    .irq          = NULL,
};

/* TODO: Parse UART config from device-tree or command-line */

void __init mfld_uart_init(void)
{
    /* Register with generic serial driver. */
    serial_register_uart(0, &mfld_uart_driver, NULL);
}

/*
 * Local variables:
 * mode: C
 * c-set-style: "BSD"
 * c-basic-offset: 4
 * indent-tabs-mode: nil
 * End:
 */
