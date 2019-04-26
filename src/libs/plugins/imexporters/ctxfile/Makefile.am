SUBDIRS=profiles

INCLUDES = -I$(top_srcdir)/src/libs \
  -I$(top_builddir)/src/libs \
  $(gwenhywfar_includes)

AM_CFLAGS=@visibility_cflags@

EXTRA_DIST=README

imexporterplugindir = $(aqbanking_plugindir)/imexporters
noinst_LTLIBRARIES=libabimexporters_ctxfile.la
imexporterplugin_DATA=ctxfile.xml

noinst_HEADERS=ctxfile_p.h ctxfile.h


libabimexporters_ctxfile_la_SOURCES=ctxfile.c

sources:
	for f in $(libabimexporters_ctxfile_la_SOURCES); do \
	  echo $(subdir)/$$f >>$(top_srcdir)/i18nsources; \
	done
	for f in $(imexporterplugin_DATA); do \
	  echo $(subdir)/$$f >>$(top_srcdir)/pdsources; \
	done


cppcheck:
	for f in $(libabimexporters_ctxfile_la_SOURCES); do \
	  cppcheck --force $$f ; \
	done


