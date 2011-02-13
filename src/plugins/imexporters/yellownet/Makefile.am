SUBDIRS=profiles

INCLUDES = -I$(aqbanking_symlinkdir) -I$(top_srcdir)/src/libs/aqbanking $(all_includes)
#EXTRA_DIST=

noinst_HEADERS=yellownet_p.h

imexporterplugindir = $(aqbanking_plugindir)/imexporters
imexporterplugin_LTLIBRARIES=yellownet.la
imexporterplugin_DATA=yellownet.xml

yellownet_la_SOURCES=yellownet.c
yellownet_la_LIBADD=$(aqbanking_internal_libs) $(gwenhywfar_libs) 
yellownet_la_LDFLAGS = -no-undefined @STRIPALL@ -module -avoid-version


sources:
	for f in $(yellownet_la_SOURCES); do \
	  echo $(subdir)/$$f >>$(top_srcdir)/i18nsources; \
	done
	for f in $(imexporterplugin_DATA); do \
	  echo $(subdir)/$$f >>$(top_srcdir)/pdsources; \
	done
