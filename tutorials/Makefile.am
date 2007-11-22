EXTRA_DIST = README tutorials.h

INCLUDES = \
  -I$(top_builddir)/headers \
  $(gwenhywfar_includes) $(qt3_includes)

noinst_PROGRAMS=\
  tutorial1 \
  tutorial2 \
  tutorial3 \
  tutorial4 \
  qtfrontend

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

# This conditional is a synonym of a (not yet existing) HAVE_QT
if MAKE_QT_CATS
qgui_internal_libs= $(top_builddir)/src/frontends/qbanking/gui/libqgui.la
else
qgui_internal_libs=
endif

qtfrontend_SOURCES=qtfrontend.cpp
qtfrontend_LDADD=\
  $(top_builddir)/src/libs/aqbanking/libaqbanking.la \
  $(qbanking_internal_libs) \
  $(gwenhywfar_libs) $(qt3_libs)


