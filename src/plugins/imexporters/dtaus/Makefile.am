SUBDIRS=profiles

INCLUDES = -I$(aqbanking_symlinkdir) $(all_includes) -I$(top_srcdir)/src/libs/aqbanking
EXTRA_DIST=README

noinst_HEADERS=dtaus_p.h


imexporterplugindir = $(aqbanking_plugindir)/imexporters
imexporterplugin_LTLIBRARIES=dtaus.la
imexporterplugin_DATA=dtaus.xml

dtaus_la_SOURCES=dtaus.c
dtaus_la_LIBADD=$(aqbanking_internal_libs) $(gwenhywfar_libs) 
dtaus_la_LDFLAGS = -no-undefined @STRIPALL@ -module -avoid-version


sources:
	for f in $(dtaus_la_SOURCES); do \
	  echo $(subdir)/$$f >>$(top_srcdir)/i18nsources; \
	done
	for f in $(imexporterplugin_DATA); do \
	  echo $(subdir)/$$f >>$(top_srcdir)/pdsources; \
	done
