INCLUDES = $(gwenhywfar_includes)
EXTRA_DIST=README

dbioplugindir=@gwenhywfar_plugins@/dbio
dbioplugin_DATA=swift.xml

dbioplugin_LTLIBRARIES=swift.la
noinst_HEADERS=\
  swift.h \
  swift_l.h \
  swift_p.h \
  swift940_l.h \
  swift940_p.h

swift_la_SOURCES=\
 swift.c \
 swift940.c

swift_la_LIBADD=@gwenhywfar_libs@
#swift_la_LDFLAGS=@STRIPALL@ -module -version-info 0:1:0
swift_la_LDFLAGS= -module -version-info 0:1:0

