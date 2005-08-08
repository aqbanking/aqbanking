INCLUDES = -I.. -I../../libs/aqbanking -I$(top_builddir)/headers $(all_includes)

DEFS=\
  -DPACKAGE_DATA_DIR=\""$(datadir)"\" \
  -DPACKAGE_LOCALE_DIR=\""$(prefix)/$(DATADIRNAME)/locale"\"

noinst_LTLIBRARIES=libwidgets.la

iheaderdir=${includedir}/g2banking
iheader_HEADERS=\
 gprogress.h \
 gsimplebox.h \
 joblist.h

noinst_HEADERS=\
 gprogress_p.h \
 gsimplebox_p.h \
 joblist_p.h

libwidgets_la_SOURCES=\
 gprogress.c \
 gsimplebox.c \
 joblist.c


sources:
	for f in $(libwidgets_la_SOURCES); do \
	  echo $(subdir)/$$f >>$(top_srcdir)/i18nsources; \
	done
