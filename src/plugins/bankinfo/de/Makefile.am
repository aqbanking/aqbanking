INCLUDES = -I$(top_builddir)/headers $(all_includes)
EXTRA_DIST=de.c de_p.h

bankinfoplugindir = $(aqbanking_plugindir)/bankinfo

bankinfoplugin_LTLIBRARIES=de.la
bankinfoplugin_DATA=de.xml

noinst_HEADERS=\
 de_p.h

de_la_SOURCES=\
 de.c

de_la_LIBADD= $(aqbanking_internal_libs) ../generic/libbankinfo.la @ktoblzcheck_libs@ $(gwenhywfar_libs)
de_la_LDFLAGS=@STRIPALL@ -module -omit-version


sources:
	for f in $(de_la_SOURCES); do \
	  echo $(subdir)/$$f >>$(top_srcdir)/i18nsources; \
	done
