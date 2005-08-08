INCLUDES = -I$(srcdir)/.. -I.. -I$(srcdir)/../widgets \
  -I$(top_builddir)/headers $(all_includes)

DEFS += \
  -DPACKAGE_DATA_DIR=\""$(datadir)"\" \
  -DPACKAGE_LOCALE_DIR=\""$(prefix)/$(DATADIRNAME)/locale"\"

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
