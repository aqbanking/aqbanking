SUBDIRS=profiles

INCLUDES = $(all_includes)
EXTRA_DIST=README

imexporterplugindir=@aqbanking_plugins@/imexporters

imexporterplugin_LTLIBRARIES=dbio.la
imexporterplugin_DATA=dbio.xml

noinst_HEADERS=\
 dbio_p.h

dbio_la_SOURCES=\
 dbio.c

dbio_la_LIBADD=@gwenhywfar_libs@ -L../../../libs/aqbanking -laqbanking
dbio_la_LDFLAGS=@STRIPALL@ -module -version-info 0:1:0

