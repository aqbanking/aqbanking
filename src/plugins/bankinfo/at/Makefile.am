INCLUDES =-I$(top_builddir)/headers $(all_includes)

noinst_LTLIBRARIES=libbankinfo_at.la
noinst_HEADERS=at_p.h
libbankinfo_at_la_SOURCES=at.c

bankinfoplugindir = $(aqbanking_plugindir)/bankinfo
bankinfoplugin_LTLIBRARIES=at.la
bankinfoplugin_DATA=at.xml

at_la_SOURCES=dummy.c
at_la_LIBADD= libbankinfo_at.la $(aqbanking_internal_libs) ../generic/libbankinfo_generic.la $(gwenhywfar_libs) 
at_la_LDFLAGS = -no-undefined @STRIPALL@ -module -omit-version


sources:
	for f in $(libbankinfo_at_la_SOURCES); do \
	  echo $(subdir)/$$f >>$(top_srcdir)/i18nsources; \
	done
