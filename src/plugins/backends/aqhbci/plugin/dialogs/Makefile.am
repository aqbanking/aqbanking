INCLUDES=-I$(top_builddir)/headers \
  -I.. -I$(srcdir)/.. \
  -I$(top_srcdir)/src/libs/aqbanking $(gwenhywfar_includes)

AM_CFLAGS=-DBUILDING_AQHBCI @visibility_cflags@

noinst_LTLIBRARIES=libaqhbci_dlgs.la

iheaderdir=${includedir}/aqhbci
iheader_HEADERS=\
  dlg_pintan.h

noinst_HEADERS=\
  dlg_pintan_p.h

libaqhbci_dlgs_la_SOURCES=\
  dlg_pintan.c


dlgdatadir=$(aqbanking_pkgdatadir)/backends/aqhbci/dialogs
dlgdata_DATA=\
  dlg_pintan.dlg

EXTRA_DIST=$(typefiles) $(dlgdata_DATA)


sources:
	for f in $(libaqhbci_dlgs_la_SOURCES); do \
	  echo $(subdir)/$$f >>$(top_srcdir)/i18nsources; \
	done
	for d in $(SUBDIRS); do \
	  $(MAKE) -C $$d sources; \
	done
