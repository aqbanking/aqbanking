SUBDIRS=profiles

INCLUDES = -I$(aqbanking_symlinkdir) -I$(top_srcdir)/src/libs/aqbanking $(all_includes)

AM_CFLAGS=@visibility_cflags@

EXTRA_DIST=README

imexporterplugindir = $(aqbanking_plugindir)/imexporters
noinst_LTLIBRARIES=libabimexporters_openhbci1.la
imexporterplugin_DATA=openhbci1.xml

noinst_HEADERS=openhbci1_p.h openhbci1.h


libabimexporters_openhbci1_la_SOURCES=openhbci1.c

sources:
	for f in $(openhbci1_la_SOURCES); do \
	  echo $(subdir)/$$f >>$(top_srcdir)/i18nsources; \
	done
	for f in $(imexporterplugin_DATA); do \
	  echo $(subdir)/$$f >>$(top_srcdir)/pdsources; \
	done
