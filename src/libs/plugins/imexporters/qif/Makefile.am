SUBDIRS=profiles

INCLUDES = -I$(top_srcdir)/src/libs/aqbanking -I$(aqbanking_symlinkdir) $(all_includes)
#EXTRA_DIST=README

imexporterplugindir = $(aqbanking_plugindir)/imexporters

noinst_LTLIBRARIES=libabimexporters_qif.la
imexporterplugin_DATA=qif.xml 

noinst_HEADERS=\
 qif_p.h

libabimexporters_qif_la_SOURCES=\
 qif.c


sources:
	for f in $(qif_la_SOURCES); do \
	  echo $(subdir)/$$f >>$(top_srcdir)/i18nsources; \
	done
	for f in $(imexporterplugin_DATA); do \
	  echo $(subdir)/$$f >>$(top_srcdir)/pdsources; \
	done
