
project('aqbanking', 'c', version : '5.99.22.0', default_options : 'cpp_std=c++11')


##################################################################################
# versioning (dont forget to set version above!)
#

ab_version_tag    ='git'
ab_so_current     = 43
ab_so_age         = 0
ab_so_revision    = 22

ab_so_effective   = ab_so_current - ab_so_age
ab_version = meson.project_version()
ab_version_array = ab_version.split('.')
ab_so_version     = '@0@.@1@.@2@'.format(ab_so_current, ab_so_age, ab_so_revision)



pkg = import('pkgconfig')


ab_package_name = meson.project_name()
ab_bugreport = 'martin@aqbanking.de'




# private AqBanking's dependencies and defines
ab_deps = []
ab_defines = []
ab_includes = []
ab_ldflags = []


cc = meson.get_compiler('c')
host_system = host_machine.system()


##################################################################################
# create system config for config.h
#

ab_system_conf = configuration_data()

ab_prefix=get_option('prefix')




##################################################################################
# check for GCC visibility
#

if cc.compiles('''int foo_def(void) __attribute__((visibility("default")));
                  int foo_hid(void) __attribute__((visibility("hidden")));
                  int foo_int(void) __attribute__((visibility("internal")));
                  int foo_pro(void) __attribute__((visibility("protected")));''',
               name : '__attribute__((visibility(...)))')
  ab_defines += ['-DGCC_WITH_VISIBILITY_ATTRIBUTE', '-fvisibility=hidden']
endif




foreach ldflag: [ '-Wl,-Bsymbolic', '-Wl,-z,relro', '-Wl,-z,now', ]
  if cc.links('int main () { return 0; }', name: ldflag, args: ldflag)
    ab_ldflags += [ ldflag ]
  endif
endforeach




##################################################################################
# plugin-searchdir
#

ab_plugin_searchdir=get_option('plugin-searchdir')
if ab_plugin_searchdir == ''
  if host_system != 'windows'
    if get_option('local-install')
      ab_plugin_searchdir = 'lib/aqbanking/plugins/' + '@0@'.format(ab_so_effective)
    else
      ab_plugin_searchdir = ab_prefix + '/' + 'lib/aqbanking/plugins/' + '@0@'.format(ab_so_effective)
    endif
  else
    ab_plugin_searchdir = 'lib/aqbanking/plugins/' + '@0@'.format(ab_so_effective)
  endif
endif




##################################################################################
# cfg-searchdir
#

ab_cfg_searchdir=get_option('cfg-searchdir')
if ab_cfg_searchdir == ''
  if host_system != 'windows'
    if get_option('local-install')
      ab_cfg_searchdir = 'etc'
    else
      ab_cfg_searchdir = ab_prefix + '/etc'
    endif
  else
    ab_cfg_searchdir = 'etc'
  endif
endif




##################################################################################
# locale-searchdir
#

ab_locale_searchdir=get_option('locale-searchdir')
if ab_locale_searchdir == ''
  if host_system != 'windows'
    if get_option('local-install')
      ab_locale_searchdir = 'share/locale'
    else
      ab_locale_searchdir = ab_prefix + '/share/locale'
    endif
  else
    ab_locale_searchdir = 'share/locale'
  endif
endif




##################################################################################
# locale-searchdir
#

ab_data_searchdir=get_option('data-searchdir')
if ab_data_searchdir == ''
  if host_system != 'windows'
    if get_option('local-install')
      ab_data_searchdir = 'share'
    else
      ab_data_searchdir = ab_prefix + '/share'
    endif
  else
    ab_data_searchdir = 'share'
  endif
endif




##################################################################################
# find libgmp
#

libgmp = cc.find_library('gmp', required : true)
if libgmp.found()
    ab_deps += libgmp
#    libgmp_inc=include_directories(libgwenhywfar.get_pkgconfig_variable('includedir'))
endif
if not cc.has_header('gmp.h')
  error ('Header "gmp.h" not available.')
endif




##################################################################################
# find libgwenhywfar
#

libgwenhywfar = dependency('gwenhywfar', version: '>=4.99.10', required : true)
if libgwenhywfar.found()
    ab_deps += libgwenhywfar
    libgwenhywfar_inc=include_directories(libgwenhywfar.get_pkgconfig_variable('includedir'))
endif




##################################################################################
# check backends
#

backend_aqhbci_use        = false
backend_aqhbci_deps       = []

backend_aqebics_use       = false
backend_aqebics_deps      = []

backend_aqofxconnect_use  = false
backend_aqofxconnect_deps = false


backends=get_option('backends')
if backends ==''
  backends='all'
endif

if backends =='all'
  backends='aqhbci aqofxconnect aqebics'
endif

backends_array=backends.split(' ')
foreach backend: backends_array
  if backend == 'aqnone'
    backend_aqnone_use = true
    ab_system_conf.set('AQBANKING_WITH_PLUGIN_BACKEND_AQNONE', 1)
  elif backend == 'aqhbci'
    backend_aqhbci_use = true
    ab_system_conf.set('AQBANKING_WITH_PLUGIN_BACKEND_AQHBCI', 1)
  elif backend == 'aqebics'
    d = dependency('xmlsec1-gnutls', version: '>=1.0.0', required : true)
    if d.found()
      backend_aqebics_deps +=d
      backend_aqebics_use = true
      ab_system_conf.set('AQBANKING_WITH_PLUGIN_BACKEND_AQEBICS', 1)
    endif
  elif backend == 'aqofxconnect'
    backend_aqofxconnect_use = true
    ab_system_conf.set('AQBANKING_WITH_PLUGIN_BACKEND_AQOFXCONNECT', 1)
  endif
endforeach




##################################################################################
# setup configuration (used for version.h, system.h
#

ab_conf = configuration_data()

# library version
ab_conf.set('AQBANKING_VERSION_MAJOR', ab_version_array[0])
ab_conf.set('AQBANKING_VERSION_MINOR', ab_version_array[1])
ab_conf.set('AQBANKING_VERSION_PATCHLEVEL', ab_version_array[2])
ab_conf.set('AQBANKING_VERSION_BUILD', ab_version_array[3])
ab_conf.set('AQBANKING_VERSION_TAG', ab_version_tag)

# version strings
ab_conf.set('AQBANKING_VERSION_FULL_STRING', ab_version)
ab_conf.set('AQBANKING_VERSION_STRING', '"@0@.@1@.@2@"'.format(ab_version_array[0], ab_version_array[1], ab_version_array[2]))

# so versions
ab_conf.set('AQBANKING_SO_CURRENT', ab_so_current)
ab_conf.set('AQBANKING_SO_AGE', ab_so_age)
ab_conf.set('AQBANKING_SO_REVISION', ab_so_revision)
ab_conf.set('AQBANKING_SO_EFFECTIVE', ab_so_effective)

# host system
ab_conf.set('OSYSTEM', host_system)
if host_system == 'windows'
  ab_conf.set('aqbanking_sys_is_windows', 1)
else
  ab_conf.set('aqbanking_sys_is_windows', 0)
endif




##################################################################################
# setup configuration (used for config.h.in->config.h
#
#

# output config.h
configure_file(output : 'config.h', configuration : ab_system_conf)




##################################################################################
# prepare some defines
#

ab_defines += [
  '-DBUILDING_AQBANKING',
  '-DHAVE_CONFIG_H',
  '-DPACKAGE="@0@"'.format(ab_package_name),
  '-DCOMPILE_DATETIME="20190307-0153"',
  '-DAQBANKING_PLUGINS="@0@"'.format(ab_plugin_searchdir),
  '-DAQBANKING_SYSCONF_DIR="@0@"'.format(ab_cfg_searchdir),
  '-DAQBANKING_DATA_DIR="@0@"'.format(ab_data_searchdir),
  '-DLOCALEDIR="@0@"'.format(ab_locale_searchdir),
]


root_includes = include_directories('.')


subdir('src')
