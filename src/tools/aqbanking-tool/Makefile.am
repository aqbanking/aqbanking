EXTRA_DIST=README SICHERHEITSHINWEISE

idatadir=${datadir}/doc/aqbanking/aqbanking-tool
idata_DATA=README SICHERHEITSHINWEISE

INCLUDES = $(gwenhywfar_includes) -I$(all_includes)
bin_PROGRAMS=aqbanking-tool

localedir = $(datadir)/locale
DEFS += -DLOCALEDIR=\"$(localedir)\"

noinst_HEADERS=\
  cbanking_p.h cbanking_l.h cbanking.h \
  progress_p.h progress_l.h globals.h

aqbanking_tool_SOURCES=main.c cbanking.c progress.c \
 listaccs.c request.c exec.c listtrans.c \
 transfer.c debitnote.c chkacc.c chktrans.c listbal.c

aqbanking_tool_LDADD = -L$(top_builddir)/src/libs/aqbanking -laqbanking $(gwenhywfar_libs) $(aqbanking_libs)

sources:
	for f in $(aqbanking_tool_la_SOURCES); do \
	  echo $(subdir)/$$f >>$(top_srcdir)/i18nsources; \
	done
	for d in $(SUBDIRS); do \
	  make -C $$d sources; \
	done

