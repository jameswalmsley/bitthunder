if config.has('BT_CONFIG_OS')
vm_sources += 'os/src/mm/bt_mm.c'
endif

if config.has('BT_CONFIG_MEM_PAGE_ALLOCATOR')
vm_sources += 'os/src/mm/bt_page.c'
endif

if config.has('BT_CONFIG_MEM_KHEAP')
vm_sources += 'os/src/mm/bt_heap.c'
endif

if config.has('BT_CONFIG_USE_VIRTUAL_ADDRESSING')
vm_sources += 'os/src/mm/bt_vm.c'
vm_sources += 'os/src/mm/bt_map.c'
endif

if config.has('BT_CONFIG_MEM_SLAB_ALLOCATOR')
vm_sources += 'os/src/mm/slab.c'
endif
