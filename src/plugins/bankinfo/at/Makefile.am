INCLUDES =-I$(top_builddir)/headers $(all_includes)
EXTRA_DIST=at.c at_p.h

bankinfoplugindir = $(aqbanking_plugindir)/bankinfo

bankinfoplugin_LTLIBRARIES=at.la
bankinfoplugin_DATA=at.xml

noinst_HEADERS=\
 at_p.h

at_la_SOURCES=\
 at.c

at_la_LIBADD=@gwenhywfar_libs@ -L../../../libs/aqbanking -laqbanking ../generic/libbankinfo.la @ktoblzcheck_libs@
at_la_LDFLAGS=@STRIPALL@ -module -omit-version


sources:
	for f in $(at_la_SOURCES); do \
	  echo $(subdir)/$$f >>$(top_srcdir)/i18nsources; \
	done
