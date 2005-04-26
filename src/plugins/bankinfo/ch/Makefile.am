INCLUDES = $(all_includes)
EXTRA_DIST=ch.c ch_p.h

bankinfoplugindir = $(aqbanking_plugindir)/bankinfo

bankinfoplugin_LTLIBRARIES=ch.la
bankinfoplugin_DATA=ch.xml

noinst_HEADERS=\
 ch_p.h

ch_la_SOURCES=\
 ch.c

ch_la_LIBADD=@gwenhywfar_libs@ -L../../../libs/aqbanking -laqbanking ../generic/libbankinfo.la @ktoblzcheck_libs@
ch_la_LDFLAGS=@STRIPALL@ -module -omit-version


sources:
	for f in $(ch_la_SOURCES); do \
	  echo $(subdir)/$$f >>$(top_srcdir)/i18nsources; \
	done
