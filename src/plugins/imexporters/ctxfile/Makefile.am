SUBDIRS=profiles

INCLUDES = -I$(top_builddir)/headers -I$(top_srcdir)/src/libs/aqbanking $(all_includes)
EXTRA_DIST=README

imexporterplugindir = $(aqbanking_plugindir)/imexporters
imexporterplugin_LTLIBRARIES=ctxfile.la
imexporterplugin_DATA=ctxfile.xml

noinst_HEADERS=ctxfile_p.h


ctxfile_la_SOURCES=ctxfile.c
ctxfile_la_LIBADD=$(aqbanking_internal_libs) $(gwenhywfar_libs) 
ctxfile_la_LDFLAGS = -no-undefined @STRIPALL@ -module -avoid-version

sources:
	for f in $(ctxfile_la_SOURCES); do \
	  echo $(subdir)/$$f >>$(top_srcdir)/i18nsources; \
	done
