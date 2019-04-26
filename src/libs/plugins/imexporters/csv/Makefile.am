SUBDIRS=profiles

INCLUDES = -I$(top_srcdir)/src/libs \
  -I$(top_builddir)/src/libs \
  $(gwenhywfar_includes)

AM_CFLAGS=@visibility_cflags@

imexporterplugindir = $(aqbanking_plugindir)/imexporters
noinst_LTLIBRARIES=libabimexporters_csv.la
imexporterplugin_DATA=csv.xml 

dialogdatadir = $(aqbanking_pkgdatadir)/imexporters/csv/dialogs
dialogdata_DATA=csv_editprofile.dlg

libabimexporters_csv_la_SOURCES=\
  csv.c \
  csv_editprofile.c

noinst_HEADERS=\
  csv_p.h \
  csv.h \
  csv_editprofile_l.h \
  csv_editprofile_p.h

EXTRA_DIST=README $(dialogdata_DATA)


sources:
	for f in $(libabimexporters_csv_la_SOURCES); do \
	  echo $(subdir)/$$f >>$(top_srcdir)/i18nsources; \
	done
	for f in $(dialogdata_DATA); do \
	  echo $(subdir)/$$f >>$(top_srcdir)/dlgsources; \
	done
	for f in $(imexporterplugin_DATA); do \
	  echo $(subdir)/$$f >>$(top_srcdir)/pdsources; \
	done


cppcheck:
	for f in $(libabimexporters_csv_la_SOURCES); do \
	  cppcheck --force $$f ; \
	done



