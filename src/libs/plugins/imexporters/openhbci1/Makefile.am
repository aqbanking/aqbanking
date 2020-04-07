SUBDIRS=profiles

AM_CPPFLAGS = -I$(top_srcdir)/src/libs \
  -I$(top_builddir)/src/libs \
  $(gwenhywfar_includes)

AM_CFLAGS=-DBUILDING_AQBANKING @visibility_cflags@

EXTRA_DIST=README

imexporterplugindir = $(aqbanking_plugindir)/imexporters
noinst_LTLIBRARIES=libabimexporters_openhbci1.la
imexporterplugin_DATA=openhbci1.xml

noinst_HEADERS=openhbci1_p.h openhbci1.h


libabimexporters_openhbci1_la_SOURCES=openhbci1.c


typefiles:

typedefs:



sources:
	for f in $(libabimexporters_openhbci1_la_SOURCES); do \
	  echo $(subdir)/$$f >>$(top_srcdir)/i18nsources; \
	done
	for f in $(imexporterplugin_DATA); do \
	  echo $(subdir)/$$f >>$(top_srcdir)/pdsources; \
	done


cppcheck:
	for f in $(libabimexporters_openhbci1_la_SOURCES); do \
	  cppcheck --force $$f ; \
	done



