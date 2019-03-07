
subdir('backends')


libabplugins_includes = []
libabplugins_includes += include_directories('..')
libabplugins_includes += libgwenhywfar_inc
libabplugins_includes += root_includes

libabplugins_defines = ab_defines


libabplugins_src = [ 'dummy.c']


libabplugins = static_library('abplugins', libabplugins_src,
                              include_directories : libabbase_includes,
                              c_args : libabplugins_defines,
                              install : false,
                              link_whole: [libabp_backends],
)

