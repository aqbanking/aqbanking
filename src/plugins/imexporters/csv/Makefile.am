SUBDIRS=profiles

INCLUDES = -I$(top_srcdir)/src/libs/aqbanking -I$(aqbanking_symlinkdir) $(all_includes)

imexporterplugindir = $(aqbanking_plugindir)/imexporters
imexporterplugin_LTLIBRARIES=csv.la
imexporterplugin_DATA=csv.xml 

dialogdatadir = $(aqbanking_pkgdatadir)/imexporters/csv/dialogs
dialogdata_DATA=csv_editprofile.dlg

csv_la_SOURCES=\
  csv.c \
  csv_editprofile.c

noinst_HEADERS=\
  csv_p.h \
  csv_editprofile_l.h csv_editprofile_p.h

EXTRA_DIST=README $(dialogdata_DATA)


csv_la_LIBADD=$(aqbanking_internal_libs) $(gwenhywfar_libs) 
csv_la_LDFLAGS = -no-undefined @STRIPALL@ -module -avoid-version

sources:
	for f in $(csv_la_SOURCES); do \
	  echo $(subdir)/$$f >>$(top_srcdir)/i18nsources; \
	done
	for f in $(dialogdata_DATA); do \
	  echo $(subdir)/$$f >>$(top_srcdir)/dlgsources; \
	done
	for f in $(imexporterplugin_DATA); do \
	  echo $(subdir)/$$f >>$(top_srcdir)/pdsources; \
	done
