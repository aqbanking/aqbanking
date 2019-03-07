

libabp_backends_includes = []
libabp_backends_includes += libgwenhywfar_inc
libabp_backends_includes += root_includes

libabp_backends_defines = ab_defines


libabp_backends_src = [ 'dummy.c']


##################################################################################
# select backends
#

libabp_backends_lw = []

# AqNone
if backend_aqnone_use
  subdir('aqnone')
  libabp_backends_lw +=libabpbe_aqnone
endif


# AqHbci
if backend_aqhbci_use
  subdir('aqhbci')
  libabp_backends_lw +=libabpbe_aqhbci
endif



##################################################################################
# build static library
#

libabp_backends = static_library('abplugins_backends', libabp_backends_src,
                              include_directories : libabp_backends_includes,
                              c_args              : libabp_backends_defines,
                              install             : false,
                              link_whole          : [libabp_backends_lw],
)

