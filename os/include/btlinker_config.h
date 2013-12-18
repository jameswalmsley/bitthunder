#include <bt_config.h>

#ifdef BT_CONFIG_LINKER_INIT_SECTION_FLASH
#define BT_LINKER_INIT_SECTION	FLASH
#define BT_LINKER_KERNEL_ENTRY	BT_CONFIG_LINKER_FLASH_START_ADDRESS
#endif

#ifdef BT_CONFIG_LINKER_FLASH_RESERVED
	#define BT_LINKER_FLASH_START_ADDRESS	BT_CONFIG_LINKER_FLASH_START_ADDRESS + BT_CONFIG_LINKER_FLASH_RESERVED
	#define BT_LINKER_FLASH_LENGTH			BT_CONFIG_LINKER_FLASH_LENGTH        - BT_CONFIG_LINKER_FLASH_RESERVED
#else
	#define BT_LINKER_FLASH_START_ADDRESS	BT_CONFIG_LINKER_FLASH_START_ADDRESS
	#define BT_LINKER_FLASH_LENGTH			BT_CONFIG_LINKER_FLASH_LENGTH
#endif


#ifdef BT_CONFIG_LINKER_INIT_SECTION_SRAM
#define BT_LINKER_INIT_SECTION	SRAM
#define BT_LINKER_KERNEL_ENTRY	BT_CONFIG_LINKER_SRAM_START_ADDRESS
#endif

#ifdef BT_CONFIG_LINKER_SRAM_RESERVED
	#define BT_LINKER_SRAM_START_ADDRESS	BT_CONFIG_LINKER_SRAM_START_ADDRESS + BT_CONFIG_LINKER_SRAM_RESERVED
	#define BT_LINKER_SRAM_LENGTH			BT_CONFIG_LINKER_SRAM_LENGTH        - BT_CONFIG_LINKER_SRAM_RESERVED
#else
	#define BT_LINKER_SRAM_START_ADDRESS	BT_CONFIG_LINKER_SRAM_START_ADDRESS
	#define BT_LINKER_SRAM_LENGTH			BT_CONFIG_LINKER_SRAM_LENGTH
#endif


#ifdef BT_CONFIG_LINKER_INIT_SECTION_RAM
#define BT_LINKER_INIT_SECTION	RAM
#ifdef BT_CONFIG_OF
#define BT_LINKER_KERNEL_ENTRY	BT_CONFIG_LINKER_RAM_START_ADDRESS + 0x8000
#else
#define BT_LINKER_KERNEL_ENTRY	BT_CONFIG_LINKER_RAM_START_ADDRESS
#endif
#endif


#ifdef BT_CONFIG_LINKER_TEXT_SECTION_FLASH
#define BT_LINKER_TEXT_SECTION	FLASH
#endif

#ifdef BT_CONFIG_LINKER_TEXT_SECTION_SRAM
#define BT_LINKER_TEXT_SECTION	SRAM
#endif

#ifdef BT_CONFIG_LINKER_TEXT_SECTION_RAM
#define BT_LINKER_TEXT_SECTION	RAM
#endif


#ifdef BT_CONFIG_LINKER_DATA_SECTION_FLASH
#define BT_LINKER_DATA_SECTION	FLASH
#endif

#ifdef BT_CONFIG_LINKER_DATA_SECTION_SRAM
#define BT_LINKER_DATA_SECTION	SRAM
#endif

#ifdef BT_CONFIG_LINKER_DATA_SECTION_RAM
#define BT_LINKER_DATA_SECTION	RAM
#endif


#ifdef BT_CONFIG_LINKER_BSS_SECTION_FLASH
#define BT_LINKER_BSS_SECTION	FLASH
#endif

#ifdef BT_CONFIG_LINKER_BSS_SECTION_SRAM
#define BT_LINKER_BSS_SECTION	SRAM
#endif

#ifdef BT_CONFIG_LINKER_BSS_SECTION_RAM
#define BT_LINKER_BSS_SECTION	RAM
#endif


#ifdef BT_CONFIG_USE_VIRTUAL_ADDRESSING
#define BT_LINKER_RAM_ADDRESS	0xC0000000
#else
#define BT_LINKER_RAM_ADDRESS	BT_CONFIG_LINKER_RAM_START_ADDRESS
#endif
