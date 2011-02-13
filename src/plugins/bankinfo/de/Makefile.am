INCLUDES = -I$(aqbanking_symlinkdir) $(all_includes) $(ktoblzcheck_includes)

noinst_HEADERS=de_p.h

bankinfoplugindir = $(aqbanking_plugindir)/bankinfo
bankinfoplugin_LTLIBRARIES=de.la
bankinfoplugin_DATA=de.xml

de_la_SOURCES=de.c
de_la_LIBADD= $(aqbanking_internal_libs) ../generic/libbankinfo_generic.la @ktoblzcheck_libs@ $(gwenhywfar_libs) 
de_la_LDFLAGS = -no-undefined @STRIPALL@ -module -avoid-version


sources:
	for f in $(de_la_SOURCES); do \
	  echo $(subdir)/$$f >>$(top_srcdir)/i18nsources; \
	done
	for f in $(bankinfoplugin_DATA); do \
	  echo $(subdir)/$$f >>$(top_srcdir)/pdsources; \
	done
