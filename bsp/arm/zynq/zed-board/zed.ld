ENTRY(reset)

_STACK_SIZE = DEFINED(_STACK_SIZE) ? _STACK_SIZE : 0x2000;
_HEAP_SIZE = DEFINED(_HEAP_SIZE) ? _HEAP_SIZE : 0x2000;

_ABORT_STACK_SIZE = DEFINED(_ABORT_STACK_SIZE) ? _ABORT_STACK_SIZE : 1024;
_SUPERVISOR_STACK_SIZE = DEFINED(_SUPERVISOR_STACK_SIZE) ? _SUPERVISOR_STACK_SIZE : 2048;

/* Define Memories in the system */
MEMORY
{
	ZYNQ_RAM0	(rwx)	: ORIGIN = 0x00000000, LENGTH = 0x00030000		/*	192		KB	*/
	/*ZYNQ_RAM0 	(rwx)	: ORIGIN = 0x00100000, LENGTH = 0x1FF00000*/		/*	511		MB	*/
	ZYNQ_RAM1 	(rwx)	: ORIGIN = 0xFFFF0000, LENGTH = 0x0000FE00		/*	 63.5	KB	*/
}


SECTIONS
{
	/*
	 *	The init section allows us to place the GIC (generic interrupt controller) vector table at
	 *	adress 0x0000
	 *
	 **/
	.init : {
		_init_begin = .;
	    KEEP(*(.init))
	} > ZYNQ_RAM0 = 0

	.bt.arch.init : {
	    __bt_arch_init_start = .;
		KEEP(*(.bt.arch.init))
		KEEP(*(.bt.arch.init.*))
		__bt_arch_init_end = .;
	} > ZYNQ_RAM0

	.bt.module.entries : {
	    __bt_module_entries_start = .;
		KEEP(*(.module_entries))
		KEEP(*(.module_entries.*))
		__bt_module_entries_end = .;
	} > ZYNQ_RAM0

    .text : {
	    *(.vectors)
		*(.boot)
		*(.text)
		*(.text.*)
		*(.gnu.linkonce.t.*)
		*(.plt)
		*(.gnu_warning)
   		*(.gcc_execpt_table)
   		*(.glue_7)
   		*(.glue_7t)
   		*(.vfp11_veneer)
   		*(.ARM.extab)
   		*(.gnu.linkonce.armextab.*)
	} > ZYNQ_RAM0

	.fini : {
	   KEEP (*(.fini))
	} > ZYNQ_RAM0

	.rodata : {
	    __rodata_start = .;
      	*(.rodata)
		*(.rodata.*)
		*(.gnu.linkonce.r.*)
		__rodata_end = .;
    } > ZYNQ_RAM0

	.rodata1 : {
   	    __rodata1_start = .;
		*(.rodata1)
		*(.rodata1.*)
		__rodata1_end = .;
	} > ZYNQ_RAM0

	.sdata2 : {
		__sdata2_start = .;
		*(.sdata2)
		*(.sdata2.*)
		*(.gnu.linkonce.s2.*)
		__sdata2_end = .;
	} > ZYNQ_RAM0

	.sbss2 : {
   	    __sbss2_start = .;
		*(.sbss2)
		*(.sbss2.*)
		*(.gnu.linkonce.sb2.*)
		__sbss2_end = .;
	} > ZYNQ_RAM0

.data : {
   __data_start = .;
   *(.data)
   *(.data.*)
   *(.gnu.linkonce.d.*)
   *(.jcr)
   *(.got)
   *(.got.plt)
   __data_end = .;
} > ZYNQ_RAM0

.data1 : {
   __data1_start = .;
   *(.data1)
   *(.data1.*)
   __data1_end = .;
} > ZYNQ_RAM1

.got : {
   *(.got)
} > ZYNQ_RAM0

.ctors : {
   __CTOR_LIST__ = .;
   ___CTORS_LIST___ = .;
   KEEP (*crtbegin.o(.ctors))
   KEEP (*(EXCLUDE_FILE(*crtend.o) .ctors))
   KEEP (*(SORT(.ctors.*)))
   KEEP (*(.ctors))
   __CTOR_END__ = .;
   ___CTORS_END___ = .;
} > ZYNQ_RAM0

.dtors : {
   __DTOR_LIST__ = .;
   ___DTORS_LIST___ = .;
   KEEP (*crtbegin.o(.dtors))
   KEEP (*(EXCLUDE_FILE(*crtend.o) .dtors))
   KEEP (*(SORT(.dtors.*)))
   KEEP (*(.dtors))
   __DTOR_END__ = .;
   ___DTORS_END___ = .;
} > ZYNQ_RAM0

.fixup : {
   __fixup_start = .;
   *(.fixup)
   __fixup_end = .;
} > ZYNQ_RAM0

.eh_frame : {
   *(.eh_frame)
} > ZYNQ_RAM0

.eh_framehdr : {
   __eh_framehdr_start = .;
   *(.eh_framehdr)
   __eh_framehdr_end = .;
} > ZYNQ_RAM0

.gcc_except_table : {
   *(.gcc_except_table)
} > ZYNQ_RAM0

.sdata : {
   __sdata_start = .;
   *(.sdata)
   *(.sdata.*)
   *(.gnu.linkonce.s.*)
   __sdata_end = .;
} > ZYNQ_RAM0

.sbss (NOLOAD) : {
   __sbss_start = .;
   *(.sbss)
   *(.sbss.*)
   *(.gnu.linkonce.sb.*)
   __sbss_end = .;
} > ZYNQ_RAM0

.tdata : {
   __tdata_start = .;
   *(.tdata)
   *(.tdata.*)
   *(.gnu.linkonce.td.*)
   __tdata_end = .;
} > ZYNQ_RAM0

.tbss : {
   __tbss_start = .;
   *(.tbss)
   *(.tbss.*)
   *(.gnu.linkonce.tb.*)
   __tbss_end = .;
} > ZYNQ_RAM0

.mmu_tbl : {
   . = ALIGN(0x4000);
   __mmu_tbl_start = .;
   *(.mmu_tbl)
   __mmu_tbl_end = .;
} > ZYNQ_RAM0

.bss (NOLOAD) : {
   __bss_start = .;
   *(.bss)
   *(.bss.*)
   *(.gnu.linkonce.b.*)
   *(COMMON)
   __bss_end = .;
} > ZYNQ_RAM0

.ARM.exidx : {
   __exidx_start = .;
   *(.ARM.exidx*)
   *(.gnu.linkonce.armexidix.*.*)
   __exidx_end = .;
} > ZYNQ_RAM0

.preinit_array : {
   __preinit_array_start = .;
   KEEP (*(SORT(.preinit_array.*)))
   KEEP (*(.preinit_array))
   __preinit_array_end = .;
} > ZYNQ_RAM0

.init_array : {
   __init_array_start = .;
   KEEP (*(SORT(.init_array.*)))
   KEEP (*(.init_array))
   __init_array_end = .;
} > ZYNQ_RAM0

.fini_array : {
   __fini_array_start = .;
   KEEP (*(SORT(.fini_array.*)))
   KEEP (*(.fini_array))
   __fini_array_end = .;
} > ZYNQ_RAM0

.ARM.attributes : {
   __ARM.attributes_start = .;
   *(.ARM.attributes)
   __ARM.attributes_end = .;
} > ZYNQ_RAM0

_SDA_BASE_ = __sdata_start + ((__sbss_end - __sdata_start) / 2 );

_SDA2_BASE_ = __sdata2_start + ((__sbss2_end - __sdata2_start) / 2 );

/* Generate Stack and Heap definitions */

.heap (NOLOAD) : {
   . = ALIGN(16);
   _heap = .;
   HeapBase = .;
   _heap_start = .;
   . += _HEAP_SIZE;
   _heap_end = .;
   HeapLimit = .;
} > ZYNQ_RAM0

.stack (NOLOAD) : {
   . = ALIGN(16);
   _stack_end = .;
   . += _STACK_SIZE;
   _stack = .;
   __stack = _stack;
   . = ALIGN(16);
   _irq_stack_end = .;
   . += _STACK_SIZE;
   __irq_stack = .;
   _supervisor_stack_end = .;
   . += _SUPERVISOR_STACK_SIZE;
   . = ALIGN(16);
   __supervisor_stack = .;
   _abort_stack_end = .;
   . += _ABORT_STACK_SIZE;
   . = ALIGN(16);
   __abort_stack = .;
} > ZYNQ_RAM0

_end = .;
}

