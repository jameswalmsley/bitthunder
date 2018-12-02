project('bitthunder', 'c',
	version : '1.0',
	default_options : ['warning_level=1'])


vm_sources = []
vm_includes = []
bt_config = configuration_data()
cfg = run_command('cat', '.config').stdout().strip().split('\n')
foreach item : cfg
	bt_config.set(item, 1)
endforeach

subdir('lib')
subdir('os')
subdir('arch/arm')
subdir('kernel')

inc = include_directories(vm_includes)


#@0@/@1@'.format(meson.current_source_dir(), mapfile)
linker_script = '@0@/build/arch/arm/bitthunder.lds'.format(meson.current_source_dir())


link_args = ['-T', linker_script, '-nostartfiles'] + meson.get_cross_property('link_args')

executable('vmthunder', vm_sources, include_directories: inc, link_args: link_args)



