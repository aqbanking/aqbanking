SUBDIRS=profiles

INCLUDES = -I$(top_srcdir)/src/libs/aqbanking -I$(top_builddir)/headers $(all_includes)

abs_top_builddir=$(shell (cd $(top_builddir) && pwd) )

EXTRA_DIST=README

noinst_HEADERS=csv_p.h

imexporterplugindir = $(aqbanking_plugindir)/imexporters
imexporterplugin_LTLIBRARIES=csv.la
imexporterplugin_DATA=csv.xml 

csv_la_SOURCES=csv.c
csv_la_LIBADD=$(aqbanking_internal_libs) $(gwenhywfar_libs) 
csv_la_LDFLAGS = -no-undefined @STRIPALL@ -module -avoid-version

sources:
	for f in $(csv_la_SOURCES); do \
	  echo $(subdir)/$$f >>$(top_srcdir)/i18nsources; \
	done
