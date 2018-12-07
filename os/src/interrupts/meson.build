pwd = meson.current_source_dir() + '/'

vm_sources += pwd + 'bt_interrupts.c'

if config.has('BT_CONFIG_INTERRUPTS_SOFTIRQ')
vm_sources += pwd + 'bt_softirq.c'
endif

if config.has('BT_CONFIG_TASKLETS')
vm_sources += pwd + 'bt_tasklets.c'
endif

