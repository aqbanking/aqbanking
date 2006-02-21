INCLUDES = -I$(srcdir)/.. -I.. -I$(srcdir)/../widgets \
  -I$(top_builddir)/headers $(all_includes)

LIBDATADIR="$(datadir)/g2banking"
DEFS += -DBUILDING_GBANKING \
  -DPACKAGE_DATA_DIR=\""$(LIBDATADIR)\"" \
  -DPACKAGE_LOCALE_DIR=\""$(datadir)/locale\""

AM_CFLAGS=@visibility_cflags@

noinst_LTLIBRARIES=libdialogs.la

#iheaderdir=${includedir}/g2banking
#iheader_HEADERS=

noinst_HEADERS=\
 gbinputbox_l.h \
 gbinputbox_p.h \
 gbmsgbox_l.h \
 gbmsgbox_p.h

libdialogs_la_SOURCES=\
 gbinputbox.c \
 gbmsgbox.c


sources:
	for f in $(libdialogs_la_SOURCES); do \
	  echo $(subdir)/$$f >>$(top_srcdir)/i18nsources; \
	done
