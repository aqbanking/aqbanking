INCLUDES = $(gwenhywfar_includes)
EXTRA_DIST=README

dbioplugindir=@gwenhywfar_plugins@/dbio

dbioplugin_LTLIBRARIES=dtaus.la
dbioplugin_DATA=dtaus.xml

noinst_HEADERS=\
 dtaus.h \
 dtaus_p.h \
 dtaus-import_p.h \
 dtaus-export_p.h

dtaus_la_SOURCES=\
 dtaus.c \
 dtaus-import.c \
 dtaus-export.c

dtaus_la_LIBADD=@gwenhywfar_libs@
dtaus_la_LDFLAGS=@STRIPALL@ -module -omit-version
