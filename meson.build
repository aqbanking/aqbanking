project('aqbanking', 'c', version : '5.99.22.0', default_options : 'cpp_std=c++11')

pkg = import('pkgconfig')

ab_version = meson.project_version()
ab_version_array = ab_version.split('.')

cc = meson.get_compiler('c')
host_system = host_machine.system()


# private AqBanking's dependencies and defines
ab_deps = []
ab_defines = []
ab_includes = []


# find libgwenhywfar
libgwenhywfar = dependency('gwenhywfar', version: '>=4.99.10', required : true)
if libgwenhywfar.found()
    ab_defines += '-DHAVE_JPEG_H'
    ab_deps += libgwenhywfar
    libgwenhywfar_inc=include_directories(libgwenhywfar.get_pkgconfig_variable('includedir'))
endif




conf = configuration_data()
conf.set('AQBANKING_MAJOR_VERSION', ab_version_array[0])
conf.set('AQBANKING_MINOR_VERSION', ab_version_array[1])
conf.set('AQBANKING_PATCH_LEVEL', ab_version_array[2])
conf.set('AQBANKING_BUILD', ab_version_array[3])

subdir('src')
