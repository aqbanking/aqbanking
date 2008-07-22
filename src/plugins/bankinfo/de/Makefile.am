INCLUDES = -I$(top_builddir)/headers $(all_includes) $(ktoblzcheck_includes)

noinst_LTLIBRARIES=libbankinfo_de.la
noinst_HEADERS=de_p.h
libbankinfo_de_la_SOURCES=de.c
libbankinfo_de_la_LIBADD= @ktoblzcheck_libs@

bankinfoplugindir = $(aqbanking_plugindir)/bankinfo
bankinfoplugin_LTLIBRARIES=de.la
bankinfoplugin_DATA=de.xml

de_la_SOURCES=dummy.c
de_la_LIBADD= libbankinfo_de.la $(aqbanking_internal_libs) ../generic/libbankinfo_generic.la @ktoblzcheck_libs@ $(gwenhywfar_libs) 
de_la_LDFLAGS = -no-undefined @STRIPALL@ -module -avoid-version


sources:
	for f in $(libbankinfo_de_la_SOURCES); do \
	  echo $(subdir)/$$f >>$(top_srcdir)/i18nsources; \
	done
