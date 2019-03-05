SUBDIRS=profiles

INCLUDES = -I$(top_srcdir)/src/libs $(gwenhywfar_includes)

AM_CFLAGS=@visibility_cflags@

EXTRA_DIST=README

noinst_HEADERS=xmldb_p.h xmldb.h

imexporterplugindir = $(aqbanking_plugindir)/imexporters
noinst_LTLIBRARIES=libabimexporters_xmldb.la
imexporterplugin_DATA=xmldb.xml

libabimexporters_xmldb_la_SOURCES=xmldb.c


sources:
	for f in $(libabimexporters_xmldb_la_SOURCES); do \
	  echo $(subdir)/$$f >>$(top_srcdir)/i18nsources; \
	done
	for f in $(imexporterplugin_DATA); do \
	  echo $(subdir)/$$f >>$(top_srcdir)/pdsources; \
	done

cppcheck:
	for f in $(libabimexporters_xmldb_la_SOURCES); do \
	  cppcheck --force $$f ; \
	done


