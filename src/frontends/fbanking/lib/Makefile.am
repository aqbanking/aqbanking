SUBDIRS=base-widgets dialogs ext-widgets

EXTRA_DIST=fbanking.pc.in

INCLUDES = \
  -I$(top_builddir)/headers \
  -I$(srcdir)/dialogs -I$(srcdir)/widgets -I$(srcdir)/views \
  -Idialogs -Iwidgets -Iviews \
  $(fox_includes) $(all_includes)

DEFS += -DBUILDING_FBANKING \
  -DPKGDATADIR=\"$(pkgdatadir)\" \
  -DFBANKING_PLUGINS=\"@fbanking_plugindir@\" \
  -DFBANKING_HELPDIR=\"@fbanking_helpdir@\"


pkgconfigdir=$(libdir)/pkgconfig
pkgconfig_DATA=fbanking.pc

iheaderdir = $(includedir)/fbanking
iheader_HEADERS=\
 banking.h \
 fbanking.h

noinst_HEADERS = \
 banking_p.h

lib_LTLIBRARIES=libfbanking.la

libfbanking_la_SOURCES=\
  banking.cpp \
  fbanking.cpp

libfbanking_la_LIBADD=\
 $(aqbanking_internal_libs) $(gwenhywfar_libs) $(fox_libs)

libfbanking_la_LDFLAGS= -no-undefined \
  -version-info @FBANKING_SO_CURRENT@:@FBANKING_SO_REVISION@:@FBANKING_SO_AGE@


