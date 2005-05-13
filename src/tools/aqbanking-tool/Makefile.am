EXTRA_DIST=README SICHERHEITSHINWEISE

INCLUDES = $(gwenhywfar_includes) -I$(all_includes)
noinst_PROGRAMS=aqbanking-tool

localedir = $(datadir)/locale
DEFS += -DLOCALEDIR=\"$(localedir)\"

noinst_HEADERS=\
  cbanking_p.h cbanking_l.h cbanking.h \
  progress_p.h progress_l.h globals.h

aqbanking_tool_SOURCES=main.c cbanking.c progress.c \
 listaccs.c request.c exec.c listtrans.c transfer.c

aqbanking_tool_LDADD = -L$(top_builddir)/src/libs/aqbanking -laqbanking $(gwenhywfar_libs) $(aqbanking_libs)

sources:

