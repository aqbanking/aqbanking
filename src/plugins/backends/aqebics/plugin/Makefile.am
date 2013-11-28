SUBDIRS=msg client dialogs

INCLUDES=-I.. -I$(top_builddir) -I$(srcdir)/client -I$(builddir)/client \
  $(gwenhywfar_includes) -I$(aqbanking_symlinkdir) $(AQEBICS_CFLAGS)

AM_CFLAGS=-DBUILDING_AQEBICS @visibility_cflags@


EXTRA_DIST=aqebics.xml.in

iheaderdir=${includedir}/aqebics
iheader_HEADERS=\
 aqebics.h

plugindir= $(aqbanking_plugindir)/providers
plugin_DATA=aqebics.xml


noinst_HEADERS=\
 aqebics_l.h

# library
lib_LTLIBRARIES=libaqebics.la
libaqebics_la_SOURCES=dummy.c
libaqebics_la_LIBADD=\
  msg/libmsg.la \
  dialogs/libdialogs.la \
  client/libclient.la $(AQEBICS_LIBS) $(gwenhywfar_libs) $(aqbanking_internal_libs) -lz

# dynamic plugin
plugin_LTLIBRARIES=aqebics.la
aqebics_la_SOURCES=aqebics-plugin.c
aqebics_la_LDFLAGS=-module -avoid-version
aqebics_la_LIBADD=libaqebics.la $(AQEBICS_LIBS) $(gwenhywfar_libs) $(aqbanking_internal_libs) -lz

sources:
	for d in $(SUBDIRS); do \
	  $(MAKE) -C $$d sources; \
	done
	for f in $(libaqebics_la_SOURCES); do \
	  echo $(subdir)/$$f >>$(top_srcdir)/i18nsources; \
	done

