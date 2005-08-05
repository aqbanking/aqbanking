INCLUDES = -I$(top_builddir)/headers $(all_includes)
EXTRA_DIST=us.c us_p.h

bankinfoplugindir = $(aqbanking_plugindir)/bankinfo

bankinfoplugin_LTLIBRARIES=us.la
bankinfoplugin_DATA=us.xml

noinst_HEADERS=\
 us_p.h

us_la_SOURCES=\
 us.c

us_la_LIBADD=@gwenhywfar_libs@ -L../../../libs/aqbanking -laqbanking ../generic/libbankinfo.la @ktoblzcheck_libs@
us_la_LDFLAGS=@STRIPALL@ -module -omit-version


sources:
	for f in $(us_la_SOURCES); do \
	  echo $(subdir)/$$f >>$(top_srcdir)/i18nsources; \
	done
