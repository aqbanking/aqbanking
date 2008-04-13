INCLUDES =-I$(top_builddir)/headers $(all_includes)

noinst_LTLIBRARIES=libbankinfo_ch.la
noinst_HEADERS=ch_p.h
libbankinfo_ch_la_SOURCES=ch.c

bankinfoplugindir = $(aqbanking_plugindir)/bankinfo
bankinfoplugin_LTLIBRARIES=ch.la
bankinfoplugin_DATA=ch.xml

ch_la_SOURCES=dummy.c
ch_la_LIBADD= libbankinfo_ch.la $(aqbanking_internal_libs) ../generic/libbankinfo_generic.la $(gwenhywfar_libs) 
ch_la_LDFLAGS = -no-undefined @STRIPALL@ -module -omit-version


sources:
	for f in $(libbankinfo_ch_la_SOURCES); do \
	  echo $(subdir)/$$f >>$(top_srcdir)/i18nsources; \
	done
