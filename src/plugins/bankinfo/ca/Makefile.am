INCLUDES = -I$(aqbanking_symlinkdir) $(all_includes)

noinst_HEADERS=ca_p.h

bankinfoplugindir = $(aqbanking_plugindir)/bankinfo
bankinfoplugin_LTLIBRARIES=ca.la
bankinfoplugin_DATA=ca.xml

ca_la_SOURCES=ca.c
ca_la_LIBADD= $(aqbanking_internal_libs) ../generic/libbankinfo_generic.la $(gwenhywfar_libs)
ca_la_LDFLAGS = -no-undefined @STRIPALL@ -module -avoid-version


sources:
	for f in $(ca_la_SOURCES); do \
	  echo $(subdir)/$$f >>$(top_srcdir)/i18nsources; \
	done
	for f in $(bankinfoplugin_DATA); do \
	  echo $(subdir)/$$f >>$(top_srcdir)/pdsources; \
	done
