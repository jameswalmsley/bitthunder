vm_includes += 'arch/arm/include'
vm_includes += 'arch/arm/include/arch/common'


#
#   Common ARM build files.
#
vm_sources += 'arch/arm/boot/head.S'
vm_sources += 'arch/arm/common/arm11cpu.c'
vm_sources += 'arch/arm/common/arm-cache.c'
vm_sources += 'arch/arm/common/crtinit.s'
vm_sources += 'arch/arm/common/cpuinit.S'
vm_sources += 'arch/arm/common/gic.c'
vm_sources += 'arch/arm/common/freertos-ca9.c'
vm_sources += 'arch/arm/common/freertos-ca9-asm.S'

if config.has('BT_CONFIG_HAS_MMU')
vm_sources += 'arch/arm/mm/v7-mmu.c'
vm_sources += 'arch/arm/mm/v7-mmu-asm.S'
endif

subdir('mach')