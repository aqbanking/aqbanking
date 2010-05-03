SUBDIRS=profiles

INCLUDES = -I$(top_builddir)/headers -I$(top_srcdir)/src/libs/aqbanking $(all_includes)
EXTRA_DIST=README

imexporterplugindir = $(aqbanking_plugindir)/imexporters
imexporterplugin_LTLIBRARIES=q43.la
imexporterplugin_DATA=q43.xml

noinst_HEADERS=q43_p.h


q43_la_SOURCES=q43.c
q43_la_LIBADD=$(aqbanking_internal_libs) $(gwenhywfar_libs) 
q43_la_LDFLAGS = -no-undefined @STRIPALL@ -module -avoid-version

sources:
	for f in $(q43_la_SOURCES); do \
	  echo $(subdir)/$$f >>$(top_srcdir)/i18nsources; \
	done
