SUBDIRS=profiles

AM_CPPFLAGS = -I$(top_srcdir)/src/libs \
  -I$(top_builddir)/src/libs \
  $(gwenhywfar_includes)

AM_CFLAGS=-DBUILDING_AQBANKING @visibility_cflags@

EXTRA_DIST=eriformat.xml

noinst_HEADERS=eri2_p.h eri2.h


imexporterplugindir = $(aqbanking_plugindir)/imexporters
noinst_LTLIBRARIES=libabimexporters_eri2.la
imexporterplugin_DATA=eri2.xml

imexporterdatadir=$(aqbanking_pkgdatadir)/imexporters/eri2
dist_imexporterdata_DATA=eriformat.xml

libabimexporters_eri2_la_SOURCES=eri2.c


typefiles:

typedefs:


sources:
	for f in $(libabimexporters_eri2_la_SOURCES); do \
	  echo $(subdir)/$$f >>$(top_srcdir)/i18nsources; \
	done
	for f in $(imexporterplugin_DATA); do \
	  echo $(subdir)/$$f >>$(top_srcdir)/pdsources; \
	done

cppcheck:
	for f in $(libabimexporters_eri2_la_SOURCES); do \
	  cppcheck --force $$f ; \
	done


