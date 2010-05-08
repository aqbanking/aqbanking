SUBDIRS=profiles

INCLUDES = -I$(top_srcdir)/src/libs/aqbanking -I$(aqbanking_symlinkdir) $(all_includes)

extra_sources=\
  sepa_exp_ccm.c

EXTRA_DIST=README $(extra_sources)

noinst_HEADERS=sepa_p.h

imexporterplugindir = $(aqbanking_plugindir)/imexporters
imexporterplugin_LTLIBRARIES=sepa.la
imexporterplugin_DATA=sepa.xml

sepa_la_SOURCES=sepa.c
sepa_la_LIBADD=$(aqbanking_internal_libs) $(gwenhywfar_libs) 
sepa_la_LDFLAGS = -no-undefined @STRIPALL@ -module -avoid-version


sources:
	for f in $(sepa_la_SOURCES); do \
	  echo $(subdir)/$$f >>$(top_srcdir)/i18nsources; \
	done
