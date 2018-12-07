#
# Meson Build file for BitThunder
#
project('bitthunder', 'c',
	version : '1.0',
	default_options : ['warning_level=3'])


find_program('git')

#
# Import the Kconfig data.
#
config = configuration_data()
cfg = run_command('cat', '.config').stdout().strip().split('\n')
foreach item : cfg
	if not item.startswith('#')
		splits = item.split('=')
		if splits.length() == 2
			config.set(splits[0], splits[1].strip('"'))
		endif
	endif
endforeach

#
# Top-Level Build variables
#
ARCH=config.get('BT_CONFIG_ARCH')
SUBARCH=config.get('BT_CONFIG_SUBARCH')

#
# Subdirs can simply add to vm_sources and vm_includes.
# Or to isolate components of the build, they can create static libs in vm_libs.
#
vm_sources = []
vm_includes = []
vm_libs = []

#
# Generate config headers
#

#bsp_config = custom_target(
#    'bt_bsp_config.h',
#    output : 'bt_bsp_config.h',
#    input : '.config',
#    command : [prog_python, '@INPUT@', '--header', '@OUTPUT@'],
#]

#
# Include the base-subdirs.
#
subdir('lib')
subdir('os')
subdir('arch/' + ARCH)
subdir('kernel')
subdir('drivers')

inc = include_directories(vm_includes)

vmthunder_core = static_library('vmthunder.core', vm_sources, include_directories: inc)

extra_libs = []

foreach lib : vm_libs 
	name = lib['name']
	sources = lib['sources']
	lib_includes = []
	lib_sources = []

	foreach source : sources
		lib_sources += lib['path'] + '/' + source
	endforeach

	if 'includes' in lib
		lib_includes = include_directories(lib['includes'])
	endif

	extra_libs += static_library(name, lib_sources, include_directories: [inc, lib_includes])
endforeach

linker_script = '@0@/build/arch/arm/bitthunder.lds'.format(meson.current_source_dir())

link_args = ['-T', linker_script, '-nostartfiles']# + meson.get_cross_property('link_args')

#
# Main vmthunder kernel executable.
#
executable('vmthunder', vm_sources, include_directories: inc, link_args: link_args, link_with: [vmthunder_core, extra_libs])
