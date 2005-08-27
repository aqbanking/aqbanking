EXTRA_DIST=README SICHERHEITSHINWEISE

idatadir=$(datadir)/doc/aqbanking/aqbanking-tool
idata_DATA=README SICHERHEITSHINWEISE

INCLUDES = $(gwenhywfar_includes) \
  -I$(top_srcdir)/src/libs/aqbanking \
  -I$(top_builddir)/headers \
  -I$(all_includes)
bin_PROGRAMS=aqbanking-tool

localedir = $(datadir)/locale
DEFS += -DLOCALEDIR=\"$(localedir)\"

noinst_HEADERS=globals.h

aqbanking_tool_SOURCES=\
 main.c \
 listaccs.c \
 request.c \
 exec.c \
 listtrans.c \
 transfer.c \
 debitnote.c \
 chkacc.c \
 chktrans.c \
 listbal.c \
 import.c \
 joblog.c

aqbanking_tool_LDADD = \
  -L$(top_builddir)/src/frontends/cbanking $(cbanking_internal_libs) \
  -L$(top_builddir)/src/libs/aqbanking $(aqbanking_libs) \
  $(gwenhywfar_libs)

sources:
	for f in $(aqbanking_tool_la_SOURCES); do \
	  echo $(subdir)/$$f >>$(top_srcdir)/i18nsources; \
	done
	for d in $(SUBDIRS); do \
	  make -C $$d sources; \
	done

