INCLUDES = $(all_includes)
EXTRA_DIST=de.c de_p.h

bankinfoplugindir=@aqbanking_plugins@/bankinfo
bankinfoplugin_dedatadir=@aqbanking_data@/plugins/bankinfo/de

DEFS=@DEFS@ -DAB_BANKINFO_DE_DATAFILE=\"@aqbanking_data@/plugins/bankinfo/de/bankinfo.csv\"

bankinfoplugin_LTLIBRARIES=de.la
bankinfoplugin_DATA=de.xml

noinst_HEADERS=\
 de_p.h

de_la_SOURCES=\
 de.c

de_la_LIBADD=@gwenhywfar_libs@ -L../../../libs/aqbanking -laqbanking @ktoblzcheck_libs@
de_la_LDFLAGS=@STRIPALL@ -module -omit-version

