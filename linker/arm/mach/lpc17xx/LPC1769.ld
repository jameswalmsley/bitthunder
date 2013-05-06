ENTRY(BT_NVIC_Reset_Handler)

_STACK_SIZE = DEFINED(_STACK_SIZE) ? _STACK_SIZE : 0x100;
_HEAP_SIZE = DEFINED(_HEAP_SIZE) ? _HEAP_SIZE : 0x1400;

/* Define Memories in the system */
MEMORY
{
	FLASH	(rwx)	: ORIGIN = 0x00000000, LENGTH = 512K
	RAM 	(rwx)	: ORIGIN = 0x10000000, LENGTH = 32K
}


SECTIONS
{
	/*
	 *	The init section allows us to place the GIC (generic interrupt controller) vector table at
	 *	adress 0x0000
	 *
	 **/

	.bt.init.vectors : {
		__bt_init_start = .;
		KEEP(*(.bt.init.vectors))
		KEEP(*(.bt.init.vectors.*))
	} > FLASH

	.bt.arch.init : {
	    __bt_arch_init_start = .;
		KEEP(*(.bt.arch.init))
		KEEP(*(.bt.arch.init.*))
		__bt_arch_init_end = .;
	} > FLASH

	.bt.arch.devices : {
	    __bt_arch_devices_start = .;
		KEEP(*(.bt.arch.devices))
		KEEP(*(.bt.arch.devices.*))
		__bt_arch_devices_end = .;
	} > FLASH

	.bt.arch.drivers : {
	    __bt_arch_drivers_start = .;
		KEEP(*(.bt.arch.drivers))
		KEEP(*(.bt.arch.drivers.*))
		__bt_arch_drivers_end = .;
	} > FLASH
	
	.bt.module.init : {
	    __bt_module_init_start = .;
		KEEP(*(.bt.module.init.*))
		KEEP(*(.bt.module.init))
		__bt_module_init_end = .;
	} > FLASH

	.bt.module.entries : {
	    __bt_module_entries_start = .;
		KEEP(*(.bt.module.entries))
		KEEP(*(.bt.module.entries.*))
		__bt_module_entries_end = .;
	} > FLASH
	
	.bt.devfs.entries : {
	    __bt_devfs_entries_start = .;
		KEEP(*(.bt.devfs.entries))
		KEEP(*(.bt.devfs.entries.*))
		__bt_devfs_entries_end = .;
	} > FLASH
	

    .text : {
	    *(.vectors)
		*(.text)
		*(.text.*)
	} > FLASH
	. = ALIGN(4);	
	_etext = .;


.data : {
	  . = ALIGN(4);
   __data_start = .;
   *(.data)
   *(.data.*)
   . = ALIGN(4);        /* Align the start of the fastrun part */
   *(.fastrun)
   *(.fastrun.*)
   . = ALIGN(4);        /* Align the end of the section */
   __data_end = .;
} > RAM AT>FLASH
_edata = .;


.bss :
{
_bss_begin = .;
__bss_start__ = _bss_begin;
*(.bss)
*(.bss*)
*(COMMON)
. = ALIGN(4);
_bss_end = .;
__bss_end__ = _bss_end;
} > RAM

/* Generate Stack and Heap definitions */

.heap (NOLOAD) : {
   . = ALIGN(16);
   _heap = .;
   HeapBase = .;
   _heap_start = .;
   . += _HEAP_SIZE;
   _heap_end = .;
   HeapLimit = .;
} > RAM

_end = .;
_stack = 0x10008000;
_stack_end = _stack - _STACK_SIZE; 
}

