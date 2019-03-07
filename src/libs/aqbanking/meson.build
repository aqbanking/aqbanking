

libabbase_includes = []
libabbase_includes += include_directories('..')
libabbase_includes += libgwenhywfar_inc

libabbase_defines = []
libabbase_defines += [
  '-DPACKAGE="@0@"'.format(ab_package_name),

subdir('gui')
subdir('backendsupport')


libabbase_src = [ 
 'account_type.c',
 'banking.c',
]


libabbase = static_library('abbase', libabbase_src,
                                     include_directories : libabbase_includes,
                                     c_args : ab_defines,
                                     install : false,
                                     link_with : libabbackendsupport,
)
