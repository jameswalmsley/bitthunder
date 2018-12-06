#
#   vmthunder.mach.zynq
#   Build recipe for Zynq Machine.
#

#
#   Empty sources list.
#
zynq_sources = []


#
#   Add all mandatory sources.
#
zynq_sources += 'zynq.c'
zynq_sources += 'slcr.c'
zynq_sources += 'headsmp.S'
zynq_sources += 'early_console.c'

#
# Add all optional sources.
#
if config.has('BT_CONFIG_MACH_ZYNQ_DEVCFG')
zynq_sources += 'devcfg.c'
endif

if config.has('BT_CONFIG_MACH_ZYNQ_GEM')
zynq_sources += 'gem.c'
endif

if config.has('BT_CONFIG_MACH_ZYNQ_GPIO')
zynq_sources += 'gpio.c'
endif

if config.has('BT_CONFIG_MACH_ZYNQ_I2C')
zynq_sources += 'i2c.c'
endif

if config.has('BT_CONFIG_MACH_ZYNQ_SDIO')
zynq_sources += 'sdio.c'
endif

if config.has('BT_CONFIG_MACH_ZYNQ_TIMER')
zynq_sources += 'timer.c'
endif

if config.has('BT_CONFIG_MACH_ZYNQ_UART')
zynq_sources += 'uart.c'
endif


#
# Add any extra includes.
#
zynq_includes = []
zynq_includes += 'arch/arm/mach/zynq'

#
# Hook this into the build.
#
vm_libs += { 'name': 'vmthunder.mach.zynq', 'path': meson.current_source_dir(), 'sources': zynq_sources, 'includes': zynq_includes }
