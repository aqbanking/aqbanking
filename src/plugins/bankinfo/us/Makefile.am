INCLUDES = -I$(aqbanking_symlinkdir) $(all_includes)

noinst_HEADERS=us_p.h

bankinfoplugindir = $(aqbanking_plugindir)/bankinfo
bankinfoplugin_LTLIBRARIES=us.la
bankinfoplugin_DATA=us.xml

us_la_SOURCES=us.c
us_la_LIBADD= $(aqbanking_internal_libs) ../generic/libbankinfo_generic.la $(gwenhywfar_libs) 
us_la_LDFLAGS = -no-undefined @STRIPALL@ -module -avoid-version


sources:
	for f in $(us_la_SOURCES); do \
	  echo $(subdir)/$$f >>$(top_srcdir)/i18nsources; \
	done
	for f in $(bankinfoplugin_DATA); do \
	  echo $(subdir)/$$f >>$(top_srcdir)/pdsources; \
	done
