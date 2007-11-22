SUBDIRS=base-widgets dialogs ext-widgets

EXTRA_DIST=fbanking.pc.in

INCLUDES = \
  -I$(top_builddir)/headers \
  -I$(srcdir)/dialogs -I$(srcdir)/widgets -I$(srcdir)/views \
  -Idialogs -Iwidgets -Iviews \
  -I$(top_srcdir)/src/libs/aqbanking++ \
  $(fox_includes) $(all_includes)

DEFS += -DBUILDING_FBANKING \
  -DPKGDATADIR=\"$(pkgdatadir)\" \
  -DFBANKING_PLUGINS=\"@fbanking_plugindir@\" \
  -DFBANKING_HELPDIR=\"@fbanking_helpdir@\"


pkgconfigdir=$(libdir)/pkgconfig
pkgconfig_DATA=fbanking.pc

iheaderdir = $(includedir)/fbanking
iheader_HEADERS=\
 fbanking.h

#noinst_HEADERS =

lib_LTLIBRARIES=libfbanking.la

libfbanking_la_SOURCES=\
  fbanking.cpp

libfbanking_la_LIBADD=\
 $(top_builddir)/src/libs/aqbanking++/libaqbankingpp.la \
 $(aqbanking_internal_libs) $(gwenhywfar_libs) $(fox_libs)

libfbanking_la_LDFLAGS= -no-undefined \
  -version-info @FBANKING_SO_CURRENT@:@FBANKING_SO_REVISION@:@FBANKING_SO_AGE@

sources:

