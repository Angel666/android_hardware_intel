OUTPUT_FORMAT("elf64-x86-64", "elf64-x86-64", "elf64-x86-64")
OUTPUT_ARCH(i386:x86-64)
ENTRY(start)
PHDRS
{
  text PT_LOAD ;
}
SECTIONS
{
  . = (((((((((261 >> 8) * 0xffff000000000000) | (261 << 39))) + ((1 << 39) / 2)) + (16 << 30)) + (1 << 30)) + (1 << 30))) + 0x100000;
  _start = .;
  .text : {
        _stext = .;
       *(.text)
       *(.fixup)
       *(.gnu.warning)
       _etext = .;
  } :text = 0x9090
  .rodata : {
       *(.rodata)
       *(.rodata.*)
  } :text
  . = ALIGN((1 << (7)));
  .data.read_mostly : {
       __start___ex_table = .;
       *(.ex_table)
       __stop___ex_table = .;
       __start___pre_ex_table = .;
       *(.ex_table.pre)
       __stop___pre_ex_table = .;
       *(.data.read_mostly)
       *(.data.rel.ro)
       *(.data.rel.ro.*)
  } :text
  .data : {
       . = ALIGN((1 << 12));
       *(.data.page_aligned)
       *(.data)
       *(.data.rel)
       *(.data.rel.*)
       CONSTRUCTORS
  } :text
  . = ALIGN((1 << 12));
  __init_begin = .;
  .init.text : {
       _sinittext = .;
       *(.init.text)
       _einittext = .;
  } :text
  .init.data : {
       *(.init.rodata)
       *(.init.rodata.str*)
       *(.init.data)
       *(.init.data.rel)
       *(.init.data.rel.*)
  } :text
  . = ALIGN(32);
  .init.setup : {
       __setup_start = .;
       *(.init.setup)
       __setup_end = .;
  } :text
  .initcall.init : {
       __initcall_start = .;
       *(.initcallpresmp.init)
       __presmp_initcall_end = .;
       *(.initcall1.init)
       __initcall_end = .;
  } :text
  .xsm_initcall.init : {
       __xsm_initcall_start = .;
       *(.xsm_initcall.init)
       __xsm_initcall_end = .;
  } :text
  . = ALIGN(((1 << 12) << 3));
  __init_end = .;
  .bss : {
       __bss_start = .;
       *(.bss.stack_aligned)
       . = ALIGN((1 << 12));
       *(.bss.page_aligned)
       *(.bss)
       . = ALIGN((1 << (7)));
       __per_cpu_start = .;
       *(.bss.percpu)
       . = ALIGN((1 << (7)));
       *(.bss.percpu.read_mostly)
       . = ALIGN((1 << (7)));
       __per_cpu_data_end = .;
  } :text
  _end = . ;
  /DISCARD/ : {
       *(.exit.text)
       *(.exit.data)
       *(.exitcall.exit)
       *(.eh_frame)
  }
  .stab 0 : { *(.stab) }
  .stabstr 0 : { *(.stabstr) }
  .stab.excl 0 : { *(.stab.excl) }
  .stab.exclstr 0 : { *(.stab.exclstr) }
  .stab.index 0 : { *(.stab.index) }
  .stab.indexstr 0 : { *(.stab.indexstr) }
  .comment 0 : { *(.comment) }
}
