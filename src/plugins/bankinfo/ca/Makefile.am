INCLUDES = -I$(top_builddir)/headers $(all_includes)
EXTRA_DIST=ca.c ca_p.h

bankinfoplugindir = $(aqbanking_plugindir)/bankinfo

bankinfoplugin_LTLIBRARIES=ca.la
bankinfoplugin_DATA=ca.xml

noinst_HEADERS=\
 ca_p.h

ca_la_SOURCES=\
 ca.c

ca_la_LIBADD= $(aqbanking_internal_libs) ../generic/libbankinfo.la $(gwenhywfar_libs) 
ca_la_LDFLAGS = -no-undefined @STRIPALL@ -module -omit-version


sources:
	for f in $(ca_la_SOURCES); do \
	  echo $(subdir)/$$f >>$(top_srcdir)/i18nsources; \
	done
