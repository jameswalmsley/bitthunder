project('bitthunder', 'c',
	version : '1.0',
	default_options : ['warning_level=3'])


vm_sources = []
vm_includes = []

subdir('lib')
subdir('os')
subdir('arch/arm')
subdir('kernel')

inc = include_directories(vm_includes)


#@0@/@1@'.format(meson.current_source_dir(), mapfile)
linker_script = '@0@/build/arch/arm/bitthunder.lds'.format(meson.current_source_dir())


link_args = ['-T', linker_script, '-nostartfiles'] + meson.get_cross_property('link_args')

executable('vmthunder', vm_sources, include_directories: inc, link_args: link_args)



