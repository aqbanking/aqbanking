
libabpbe_aqnone_includes = []
libabpbe_aqnone_includes += include_directories('..')
libabpbe_aqnone_includes += libgwenhywfar_inc
libabpbe_aqnone_includes += root_includes
libabpbe_aqnone_includes += aqbanking_includes

libabpbe_aqnone_defines = ab_defines


libabpbe_aqnone_src = [ 'provider.c']


libabpbe_aqnone = static_library('abpbe_aqnone', libabpbe_aqnone_src,
                              include_directories : libabpbe_aqnone_includes,
                              c_args : libabpbe_aqnone_defines,
                              install : false,
)

