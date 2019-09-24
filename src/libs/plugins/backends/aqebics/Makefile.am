SUBDIRS=msg client dialogs control requests

INCLUDES = -I$(top_srcdir)/src/libs -I$(top_builddir)/src/libs $(gwenhywfar_includes) -I$(srcdir)/.. $(AQEBICS_CFLAGS)

AM_CFLAGS=-DBUILDING_AQBANKING @visibility_cflags@


EXTRA_DIST=aqebics.xml.in

#iheaderdir=${includedir}/aqebics
#iheader_HEADERS=

plugindir= $(aqbanking_plugindir)/providers
plugin_DATA=aqebics.xml


noinst_HEADERS=\
 aqebics.h \
 aqebics_l.h

# library
noinst_LTLIBRARIES=libaqebics.la
libaqebics_la_SOURCES=dummy.c
libaqebics_la_LIBADD=\
  msg/libmsg.la \
  dialogs/libdialogs.la \
  client/libclient.la \
  requests/libaqebics_requests.la \
  control/libebicscontrol.la -lz $(AQEBICS_LIBS)


sources:
	for d in $(SUBDIRS); do \
	  $(MAKE) -C $$d sources; \
	done
	for f in $(libaqebics_la_SOURCES); do \
	  echo $(subdir)/$$f >>$(top_srcdir)/i18nsources; \
	done


cppcheck:
	for f in $(libaqebics_la_SOURCES); do \
	  cppcheck --force $$f ; \
	done
	for d in $(SUBDIRS); do \
	  $(MAKE) -C $$d cppcheck; \
	done

types:

typedefs:
