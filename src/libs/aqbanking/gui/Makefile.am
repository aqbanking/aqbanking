INCLUDES = -I$(srcdir)/../.. -I$(srcdir)/.. -I$(top_builddir)/src/libs $(gwenhywfar_includes)

AM_CFLAGS=-DBUILDING_AQBANKING @visibility_cflags@

noinst_LTLIBRARIES=libabgui.la

EXTRA_DIST=$(typefiles)

noinst_HEADERS=\
  abgui_p.h


iheaderdir=@aqbanking_headerdir_am@/aqbanking/gui
iheader_HEADERS=\
  abgui.h

libabgui_la_SOURCES=\
  abgui.c


sources:
	for f in $(libabgui_la_SOURCES); do \
	  echo $(subdir)/$$f >>$(top_srcdir)/i18nsources; \
	done

types:

typedefs:

built_sources:



cppcheck:
	for f in $(libabgui_la_SOURCES); do \
	  cppcheck --force $$f ; \
	done

