SUBDIRS=profiles

INCLUDES = -I$(aqbanking_symlinkdir) -I$(top_srcdir)/src/libs/aqbanking $(all_includes)
EXTRA_DIST=eriformat.xml

noinst_HEADERS=eri2_p.h


imexporterplugindir = $(aqbanking_plugindir)/imexporters
imexporterplugin_LTLIBRARIES=eri2.la
imexporterplugin_DATA=eri2.xml

imexporterdatadir=$(aqbanking_pkgdatadir)/imexporters/eri2
dist_imexporterdata_DATA=eriformat.xml

eri2_la_SOURCES=eri2.c
eri2_la_LIBADD=$(aqbanking_internal_libs) $(gwenhywfar_libs) 
eri2_la_LDFLAGS = -no-undefined @STRIPALL@ -module -avoid-version

sources:
	for f in $(eri2_la_SOURCES); do \
	  echo $(subdir)/$$f >>$(top_srcdir)/i18nsources; \
	done
	for f in $(imexporterplugin_DATA); do \
	  echo $(subdir)/$$f >>$(top_srcdir)/pdsources; \
	done
