SUBDIRS=profiles

AM_CPPFLAGS = -I$(top_srcdir)/src/libs -I$(top_builddir)/src/libs $(gwenhywfar_includes)
#EXTRA_DIST=README

imexporterplugindir = $(aqbanking_plugindir)/imexporters

noinst_LTLIBRARIES=libabimexporters_qif.la
imexporterplugin_DATA=qif.xml 

noinst_HEADERS=\
 qif_p.h

libabimexporters_qif_la_SOURCES=\
 qif.c



typefiles:

typedefs:


sources:
	for f in $(libabimexporters_qif_la_SOURCES); do \
	  echo $(subdir)/$$f >>$(top_srcdir)/i18nsources; \
	done
	for f in $(imexporterplugin_DATA); do \
	  echo $(subdir)/$$f >>$(top_srcdir)/pdsources; \
	done


cppcheck:
	for f in $(libabimexporters_qif_la_SOURCES); do \
	  cppcheck --force $$f ; \
	done


