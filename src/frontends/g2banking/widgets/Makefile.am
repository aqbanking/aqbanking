INCLUDES = -I$(srcdir)/.. -I.. \
  -I$(top_srcdir)/src/libs/aqbanking \
  -I$(top_builddir)/headers $(all_includes)

LIBDATADIR="$(datadir)/g2banking"
DEFS += -DBUILDING_GBANKING \
  -DPACKAGE_DATA_DIR=\""$(LIBDATADIR)\"" \
  -DPACKAGE_LOCALE_DIR=\""$(datadir)/locale\""

AM_CFLAGS=@visibility_cflags@

noinst_LTLIBRARIES=libwidgets.la

iheaderdir=${includedir}/g2banking
iheader_HEADERS=\
 gbjoblist.h \
 gbprofilelist.h

noinst_HEADERS=\
 gbprogress_l.h \
 gbprogress_p.h \
 gbsimplebox_l.h \
 gbsimplebox_p.h \
 gbjoblist_p.h \
 gbprofilelist_p.h

libwidgets_la_SOURCES=\
 gbprogress.c \
 gbsimplebox.c \
 gbjoblist.c \
 gbprofilelist.c


sources:
	for f in $(libwidgets_la_SOURCES); do \
	  echo $(subdir)/$$f >>$(top_srcdir)/i18nsources; \
	done
