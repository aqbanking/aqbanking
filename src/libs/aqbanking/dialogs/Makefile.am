INCLUDES=-I$(top_builddir)/headers -I../  @all_includes@

AM_CFLAGS=-DBUILDING_AQBANKING @visibility_cflags@

noinst_LTLIBRARIES=libabdialogs.la

iheaderdir=${includedir}/aqbanking
iheader_HEADERS=\
  dlg_importer.h

noinst_HEADERS=\
  dlg_importer_p.h

libabdialogs_la_SOURCES=\
  dlg_importer.c


dlgdatadir=$(aqbanking_pkgdatadir)/dialogs
dlgdata_DATA=\
  dlg_importer.dlg

EXTRA_DIST=$(typefiles) $(dlgdata_DATA)


sources:
	for f in $(libaqhbci_la_SOURCES); do \
	  echo $(subdir)/$$f >>$(top_srcdir)/i18nsources; \
	done
	for d in $(SUBDIRS); do \
	  $(MAKE) -C $$d sources; \
	done
