INCLUDES = -I$(top_builddir)/headers $(all_includes)

noinst_LTLIBRARIES=libbankinfo_us.la
noinst_HEADERS=us_p.h
libbankinfo_us_la_SOURCES=us.c

bankinfoplugindir = $(aqbanking_plugindir)/bankinfo
bankinfoplugin_LTLIBRARIES=us.la
bankinfoplugin_DATA=us.xml

us_la_SOURCES=dummy.c
us_la_LIBADD= libbankinfo_us.la $(aqbanking_internal_libs) ../generic/libbankinfo_generic.la $(gwenhywfar_libs) 
us_la_LDFLAGS = -no-undefined @STRIPALL@ -module -avoid-version


sources:
	for f in $(us_la_SOURCES); do \
	  echo $(subdir)/$$f >>$(top_srcdir)/i18nsources; \
	done
