INCLUDES =-I$(aqbanking_symlinkdir) $(all_includes)

noinst_HEADERS=ch_p.h

bankinfoplugindir = $(aqbanking_plugindir)/bankinfo
bankinfoplugin_LTLIBRARIES=ch.la
bankinfoplugin_DATA=ch.xml

ch_la_SOURCES=ch.c
ch_la_LIBADD= $(aqbanking_internal_libs) ../generic/libbankinfo_generic.la $(gwenhywfar_libs) 
ch_la_LDFLAGS = -no-undefined @STRIPALL@ -module -avoid-version


sources:
	for f in $(ch_la_SOURCES); do \
	  echo $(subdir)/$$f >>$(top_srcdir)/i18nsources; \
	done
	for f in $(bankinfoplugin_DATA); do \
	  echo $(subdir)/$$f >>$(top_srcdir)/pdsources; \
	done
