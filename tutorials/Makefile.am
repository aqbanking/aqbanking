EXTRA_DIST = README tutorials.h

INCLUDES = \
  -I$(aqbanking_symlinkdir) \
  $(gwenhywfar_includes) $(qt3_includes)

noinst_PROGRAMS=\
  tutorial1 \
  tutorial2 \
  tutorial3 \
  tutorial4

tutorial1_SOURCES=tutorial1.c
tutorial1_LDADD=\
  $(top_builddir)/src/libs/aqbanking/libaqbanking.la $(gwenhywfar_libs)

tutorial2_SOURCES=tutorial2.c
tutorial2_LDADD=\
  $(top_builddir)/src/libs/aqbanking/libaqbanking.la $(gwenhywfar_libs)

tutorial3_SOURCES=tutorial3.c
tutorial3_LDADD=\
  $(top_builddir)/src/libs/aqbanking/libaqbanking.la $(gwenhywfar_libs)

tutorial4_SOURCES=tutorial4.c
tutorial4_LDADD=\
  $(top_builddir)/src/libs/aqbanking/libaqbanking.la $(gwenhywfar_libs)

