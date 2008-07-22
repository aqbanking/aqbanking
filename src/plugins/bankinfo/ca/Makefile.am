INCLUDES = -I$(top_builddir)/headers $(all_includes)

noinst_LTLIBRARIES=libbankinfo_ca.la
noinst_HEADERS=ca_p.h
libbankinfo_ca_la_SOURCES=ca.c

bankinfoplugindir = $(aqbanking_plugindir)/bankinfo
bankinfoplugin_LTLIBRARIES=ca.la
bankinfoplugin_DATA=ca.xml

ca_la_SOURCES=dummy.c
ca_la_LIBADD= libbankinfo_ca.la $(aqbanking_internal_libs) ../generic/libbankinfo_generic.la $(gwenhywfar_libs)
ca_la_LDFLAGS = -no-undefined @STRIPALL@ -module -avoid-version


sources:
	for f in $(libbankinfo_ca_la_SOURCES); do \
	  echo $(subdir)/$$f >>$(top_srcdir)/i18nsources; \
	done
