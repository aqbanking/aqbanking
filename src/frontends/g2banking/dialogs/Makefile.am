INCLUDES = -I$(srcdir)/.. -I.. -I$(srcdir)/../widgets \
  -I$(top_builddir)/headers $(all_includes)

LIBDATADIR="$(datadir)/g2banking"
DEFS += -DBUILDING_GBANKING \
  -DPACKAGE_DATA_DIR=\""$(LIBDATADIR)\"" \
  -DPACKAGE_LOCALE_DIR=\""$(datadir)/locale\""

AM_CFLAGS=@visibility_cflags@

noinst_LTLIBRARIES=libdialogs.la

iheaderdir=${includedir}/g2banking
iheader_HEADERS=\
 ginputbox.h \
 gmsgbox.h

noinst_HEADERS=\
 ginputbox_p.h \
 gmsgbox_p.h

libdialogs_la_SOURCES=\
 ginputbox.c \
 gmsgbox.c


sources:
	for f in $(libdialogs_la_SOURCES); do \
	  echo $(subdir)/$$f >>$(top_srcdir)/i18nsources; \
	done
