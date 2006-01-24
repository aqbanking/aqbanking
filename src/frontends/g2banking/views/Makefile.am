INCLUDES = -I$(srcdir)/.. -I.. -I$(srcdir)/../widgets \
  -I$(top_srcdir)/src/libs/aqbanking \
  -I$(top_builddir)/headers $(all_includes)

LIBDATADIR="$(datadir)/g2banking"
DEFS += -DBUILDING_GBANKING \
  -DPACKAGE_DATA_DIR=\""$(LIBDATADIR)\"" \
  -DPACKAGE_LOCALE_DIR=\""$(datadir)/locale\""

AM_CFLAGS=@visibility_cflags@

noinst_LTLIBRARIES=libviews.la

iheaderdir=${includedir}/g2banking
iheader_HEADERS=\
 jobview.h

noinst_HEADERS=\
 jobview_p.h

libviews_la_SOURCES=\
 jobview.c


sources:
	for f in $(libviews_la_SOURCES); do \
	  echo $(subdir)/$$f >>$(top_srcdir)/i18nsources; \
	done
