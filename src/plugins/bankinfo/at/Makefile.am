INCLUDES =-I$(aqbanking_symlinkdir) $(all_includes)

noinst_HEADERS=at_p.h

bankinfoplugindir = $(aqbanking_plugindir)/bankinfo
bankinfoplugin_LTLIBRARIES=at.la
bankinfoplugin_DATA=at.xml

at_la_SOURCES=at.c
at_la_LIBADD=$(aqbanking_internal_libs) ../generic/libbankinfo_generic.la $(gwenhywfar_libs) 
at_la_LDFLAGS= -no-undefined @STRIPALL@ -module -avoid-version


sources:
	for f in $(at_la_SOURCES); do \
	  echo $(subdir)/$$f >>$(top_srcdir)/i18nsources; \
	done
	for f in $(bankinfoplugin_DATA); do \
	  echo $(subdir)/$$f >>$(top_srcdir)/pdsources; \
	done
