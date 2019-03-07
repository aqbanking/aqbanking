

libabbase_includes = []
libabbase_includes += include_directories('..')
libabbase_includes += libgwenhywfar_inc
libabbase_includes += root_includes

libabbase_defines = ab_defines



configure_file(input : 'version.h.in', output : 'version.h', configuration : ab_conf)
configure_file(input : 'system.h.in', output : 'system.h', configuration : ab_conf)



subdir('gui')
subdir('backendsupport')
subdir('types')
subdir('dialogs')


libabbase_src = [ 
 'account_type.c',
 'banking.c',
]



libabbase = static_library('abbase', libabbase_src,
                                     include_directories : libabbase_includes,
                                     c_args : libabbase_defines,
                                     install : false,
#                                     gnu_symbol_visibility : 'hidden',
                                     link_with: [libabbackendsupport, libabdialogs, libabgui, libabtypes],
)

