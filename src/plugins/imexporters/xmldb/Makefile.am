SUBDIRS=profiles

INCLUDES = -I$(top_srcdir)/src/libs/aqbanking -I$(aqbanking_symlinkdir) $(all_includes)

EXTRA_DIST=README

noinst_HEADERS=xmldb_p.h

imexporterplugindir = $(aqbanking_plugindir)/imexporters
imexporterplugin_LTLIBRARIES=xmldb.la
imexporterplugin_DATA=xmldb.xml

xmldb_la_SOURCES=xmldb.c
xmldb_la_LIBADD=$(aqbanking_internal_libs) $(gwenhywfar_libs) 
xmldb_la_LDFLAGS = -no-undefined @STRIPALL@ -module -avoid-version


sources:
	for f in $(xmldb_la_SOURCES); do \
	  echo $(subdir)/$$f >>$(top_srcdir)/i18nsources; \
	done
	for f in $(imexporterplugin_DATA); do \
	  echo $(subdir)/$$f >>$(top_srcdir)/pdsources; \
	done
