SUBDIRS=profiles data

INCLUDES = -I$(top_srcdir)/src/libs \
  -I$(top_builddir)/src/libs \
  $(gwenhywfar_includes)

AM_CFLAGS=@visibility_cflags@

EXTRA_DIST=README

noinst_HEADERS=xml_p.h xml.h

imexporterplugindir = $(aqbanking_plugindir)/imexporters
noinst_LTLIBRARIES=libabimexporters_xml.la
imexporterplugin_DATA=xml.xml

libabimexporters_xml_la_SOURCES=xml.c



sources:
	for f in $(libabimexporters_xml_la_SOURCES); do \
	  echo $(subdir)/$$f >>$(top_srcdir)/i18nsources; \
	done
	for f in $(imexporterplugin_DATA); do \
	  echo $(subdir)/$$f >>$(top_srcdir)/pdsources; \
	done

cppcheck:
	for f in $(libabimexporters_xml_la_SOURCES); do \
	  cppcheck --force $$f ; \
	done


