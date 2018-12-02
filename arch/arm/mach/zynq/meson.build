#
#
#

pwd = meson.current_source_dir() + '/'

vm_sources += pwd + 'zynq.c'
vm_sources += pwd + 'slcr.c'
vm_sources += pwd + 'headsmp.S'
vm_sources += pwd + 'early_console.c'


if bt_config.has('BT_CONFIG_MACH_ZYNQ_DEVCFG=y') == 1
vm_sources += pwd + 'devcfg.c'
endif
