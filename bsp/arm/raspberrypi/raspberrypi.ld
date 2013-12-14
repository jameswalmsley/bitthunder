ENTRY(reset)

_STACK_SIZE = DEFINED(_STACK_SIZE) ? _STACK_SIZE : 0x2000;
_HEAP_SIZE = DEFINED(_HEAP_SIZE) ? _HEAP_SIZE : 0x100000;

_ABORT_STACK_SIZE = DEFINED(_ABORT_STACK_SIZE) ? _ABORT_STACK_SIZE : 2048;
_SUPERVISOR_STACK_SIZE = DEFINED(_SUPERVISOR_STACK_SIZE) ? _SUPERVISOR_STACK_SIZE : 2048;

MEMORY
{
	RESERVED	(r)		: ORIGIN = 0x00000000, LENGTH = 32K
	INIT_RAM	(rwx)	: ORIGIN = 0x00008000, LENGTH = 32K
	RAM			(rwx)	: ORIGIN = 0x00010000, LENGTH = 128M
}

SECTIONS {
    /*
	 * Our init section allows us to place the bootstrap code at address 0x8000
	 *
	 * This is where the Graphics processor forces the ARM to start execution.
	 * However the interrupt vector code remains at 0x0000, and so we must copy the correct
	 * branch instructions to 0x0000 - 0x001C in order to get the processor to handle interrupts.
	 *
	 */

	.init : {
		KEEP(*(.init))
		KEEP(*(.init.*))
	} > INIT_RAM

	.bt.arch.init : {
	    __bt_arch_init_start = .;
		KEEP(*(.bt.arch.init))
		KEEP(*(.bt.arch.init.*))
		__bt_arch_init_end = .;
	} > INIT_RAM

	.bt.arch.devices : {
	    __bt_arch_devices_start = .;
		KEEP(*(.bt.arch.devices))
		KEEP(*(.bt.arch.devices.*))
		__bt_arch_devices_end = .;
	} > INIT_RAM

	.bt.arch.drivers : {
	    __bt_arch_drivers_start = .;
		KEEP(*(.bt.arch.drivers))
		KEEP(*(.bt.arch.drivers.*))
		__bt_arch_drivers_end = .;
	} > INIT_RAM

	.bt.module.init : {
	    __bt_module_init_start = .;
		KEEP(*(.bt.module.init.*))
		KEEP(*(.bt.module.init))
		__bt_module_init_end = .;
	} > INIT_RAM

	.bt.devfs.entries : {
	    __bt_devfs_entries_start = .;
		KEEP(*(.bt.devfs.entries))
		KEEP(*(.bt.devfs.entries.*))
		__bt_devfs_entries_end = .;
	} > INIT_RAM

	/**
 	 *	This is the main code section, it is essentially of unlimited size. (128Mb).
	 *
	 **/
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
	} > RAM

	/*
	* Next we put the data.
	*/
	.data : {
		*(.data)
	} > RAM

	.bss :
	{
		__bss_start = .;
		*(.bss)
		*(.bss.*)
		__bss_end = .;
	} > RAM


	.heap (NOLOAD) : {
   	    . = ALIGN(16);
		_heap = .;
		HeapBase = .;
		_heap_start = .;
		. += _HEAP_SIZE;
		_heap_end = .;
		HeapLimit = .;
	} > RAM

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
    } > RAM


_end = .;

}

