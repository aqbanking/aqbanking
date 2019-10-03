SUBDIRS=profiles

AM_CPPFLAGS = -I$(top_srcdir)/src/libs \
  -I$(top_builddir)/src/libs \
  $(gwenhywfar_includes)

AM_CFLAGS=-DBUILDING_AQBANKING @visibility_cflags@

EXTRA_DIST=README


noinst_HEADERS=swift_p.h swift.h


imexporterplugindir = $(aqbanking_plugindir)/imexporters
noinst_LTLIBRARIES=libabimexporters_swift.la
imexporterplugin_DATA=swift.xml 

libabimexporters_swift_la_SOURCES=swift.c


sources:
	for f in $(libabimexporters_swift_la_SOURCES); do \
	  echo $(subdir)/$$f >>$(top_srcdir)/i18nsources; \
	done
	for f in $(imexporterplugin_DATA); do \
	  echo $(subdir)/$$f >>$(top_srcdir)/pdsources; \
	done


cppcheck:
	for f in $(libabimexporters_swift_la_SOURCES); do \
	  cppcheck --force $$f ; \
	done


