INCLUDES = -I$(srcdir)/.. -I.. \
  -I$(top_srcdir)/src/libs/aqbanking \
  -I$(top_builddir)/headers $(all_includes)

LIBDATADIR="$(datadir)/g2banking"
DEFS += \
  -DPACKAGE_DATA_DIR=\""$(LIBDATADIR)\"" \
  -DPACKAGE_LOCALE_DIR=\""$(datadir)/locale\""

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
