INCLUDES =-I$(top_builddir)/headers $(all_includes)
EXTRA_DIST=ch.c ch_p.h

bankinfoplugindir = $(aqbanking_plugindir)/bankinfo

bankinfoplugin_LTLIBRARIES=ch.la
bankinfoplugin_DATA=ch.xml

noinst_HEADERS=\
 ch_p.h

ch_la_SOURCES=\
 ch.c

ch_la_LIBADD= $(aqbanking_internal_libs) ../generic/libbankinfo.la $(gwenhywfar_libs) 
ch_la_LDFLAGS = -no-undefined @STRIPALL@ -module -omit-version


sources:
	for f in $(ch_la_SOURCES); do \
	  echo $(subdir)/$$f >>$(top_srcdir)/i18nsources; \
	done
