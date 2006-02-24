EXTRA_DIST = README tutorial.h

INCLUDES = \
  -I$(top_builddir)/headers \
  $(gwenhywfar_includes)

noinst_PROGRAMS=\
  tutorial1 \
  tutorial2 \
  tutorial3

tutorial1_SOURCES=tutorial1.c
tutorial1_LDADD=\
  $(top_builddir)/src/libs/aqbanking/libaqbanking.la \
  $(top_builddir)/src/frontends/cbanking/libcbanking.la

tutorial2_SOURCES=tutorial2.c
tutorial2_LDADD=\
  $(top_builddir)/src/libs/aqbanking/libaqbanking.la \
  $(top_builddir)/src/frontends/cbanking/libcbanking.la

tutorial3_SOURCES=tutorial3.c
tutorial3_LDADD=\
  $(top_builddir)/src/libs/aqbanking/libaqbanking.la \
  $(top_builddir)/src/frontends/cbanking/libcbanking.la


