
libabbackendsupport_includes = []
libabbackendsupport_includes += include_directories('../../')
libabbackendsupport_includes += libgwenhywfar_inc
libabbackendsupport_includes += root_includes


libabbackendsupport_defines = ab_defines


libabbackendsupport_gensrc = [
  'jobqueue.c',
  'accountqueue.c',
  'userqueue.c',
  'providerqueue.c',
  'queue.c',
  'account.c',
  'user.c',
]

libabbackendsupport_src = [ libabbackendsupport_gensrc,
  'siotlsext.c',
  'httpsession.c',
  'msgengine.c',
  'provider.c',
  'bankinfoplugin.c',
  'imexporter.c',
]


libabbackendsupport = static_library('abbackendsupport', libabbackendsupport_src,
                                     include_directories : libabbackendsupport_includes,
                                     c_args : libabbackendsupport_defines,
                                     install : false
)



