SUBDIRS=libaqfints banking

AM_CPPFLAGS = -I$(srcdir) -I$(top_srcdir)/src/libs -I$(top_builddir)/src/libs $(gwenhywfar_includes) -I$(srcdir)/..

AM_CFLAGS=-DBUILDING_AQBANKING @visibility_cflags@

EXTRA_DIST=aqfints.xml.in

plugindir=@aqbanking_plugindir@/providers
plugin_DATA=aqfints.xml

noinst_HEADERS=

# static plugin library
noinst_LTLIBRARIES=libaqfintsplugin.la 
libaqfintsplugin_la_SOURCES=dummy.c


libaqfintsplugin_la_LIBADD=\
  libaqfints/libaqfints.la \
  banking/libaqfintsbanking.la



sources:
	for f in $(libaqfintsplugin_la_SOURCES); do \
	  echo $(subdir)/$$f >>$(top_srcdir)/i18nsources; \
	done
	for f in $(plugin_DATA); do \
	  echo $(subdir)/$$f >>$(top_srcdir)/pdsources; \
	done
	for d in $(SUBDIRS); do \
	  $(MAKE) -C $$d sources; \
	done

built_sources:


cppcheck:
	for f in $(libaqfintsplugin_la_SOURCES); do \
	  cppcheck --force $$f ; \
	done
	for d in $(SUBDIRS); do \
	  $(MAKE) -C $$d cppcheck; \
	done


typefiles:
	for d in $(SUBDIRS); do \
	  $(MAKE) -C $$d typefiles; \
	done

typedefs:
	for d in $(SUBDIRS); do \
	  $(MAKE) -C $$d typedefs; \
	done


