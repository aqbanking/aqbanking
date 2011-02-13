SUBDIRS=profiles

INCLUDES = -I$(top_srcdir)/src/libs/aqbanking -I$(aqbanking_symlinkdir) $(all_includes)
#EXTRA_DIST=README

imexporterplugindir = $(aqbanking_plugindir)/imexporters

imexporterplugin_LTLIBRARIES=qif.la
imexporterplugin_DATA=qif.xml 

noinst_HEADERS=\
 qif_p.h

qif_la_SOURCES=\
 qif.c

qif_la_LIBADD=$(aqbanking_internal_libs) $(gwenhywfar_libs) 
qif_la_LDFLAGS = -no-undefined @STRIPALL@ -module -avoid-version


sources:
	for f in $(qif_la_SOURCES); do \
	  echo $(subdir)/$$f >>$(top_srcdir)/i18nsources; \
	done
	for f in $(imexporterplugin_DATA); do \
	  echo $(subdir)/$$f >>$(top_srcdir)/pdsources; \
	done
