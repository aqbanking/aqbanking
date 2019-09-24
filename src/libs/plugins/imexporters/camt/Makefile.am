SUBDIRS=profiles

INCLUDES = -I$(top_srcdir)/src/libs \
  -I$(top_builddir)/src/libs \
  $(gwenhywfar_includes)

AM_CFLAGS=-DBUILDING_AQBANKING @visibility_cflags@

extra_sources=\
  camt52_001_02.c


EXTRA_DIST=$(extra_sources)

noinst_HEADERS=camt_p.h camt.h

imexporterplugindir = $(aqbanking_plugindir)/imexporters
noinst_LTLIBRARIES=libabimexporters_camt.la
imexporterplugin_DATA=camt.xml

libabimexporters_camt_la_SOURCES=camt.c


sources:
	for f in $(libabimexporters_camt_la_SOURCES); do \
	  echo $(subdir)/$$f >>$(top_srcdir)/i18nsources; \
	done
	for f in $(imexporterplugin_DATA); do \
	  echo $(subdir)/$$f >>$(top_srcdir)/pdsources; \
	done


cppcheck:
	for f in $(libabimexporters_camt_la_SOURCES); do \
	  cppcheck --force $$f ; \
	done

