
subdir('aqnone')


libabp_backends_includes = []
#libabp_backends_includes += include_directories('..')
libabp_backends_includes += libgwenhywfar_inc
libabp_backends_includes += root_includes

libabp_backends_defines = ab_defines


libabp_backends_src = [ 'dummy.c']


libabp_backends = static_library('abplugins_backends', libabp_backends_src,
                              include_directories : libabp_backends_includes,
                              c_args : libabp_backends_defines,
                              install : false,
                              link_whole: [libabpbe_aqnone],
)

