SUBDIRS=profiles

INCLUDES = -I$(aqbanking_symlinkdir) -I$(top_srcdir)/src/libs/aqbanking $(all_includes)
EXTRA_DIST=README

imexporterplugindir = $(aqbanking_plugindir)/imexporters
imexporterplugin_LTLIBRARIES=openhbci1.la
imexporterplugin_DATA=openhbci1.xml

noinst_HEADERS=openhbci1_p.h


openhbci1_la_SOURCES=openhbci1.c
openhbci1_la_LIBADD=$(aqbanking_internal_libs) $(gwenhywfar_libs) 
openhbci1_la_LDFLAGS = -no-undefined @STRIPALL@ -module -avoid-version

sources:
	for f in $(openhbci1_la_SOURCES); do \
	  echo $(subdir)/$$f >>$(top_srcdir)/i18nsources; \
	done
	for f in $(imexporterplugin_DATA); do \
	  echo $(subdir)/$$f >>$(top_srcdir)/pdsources; \
	done
