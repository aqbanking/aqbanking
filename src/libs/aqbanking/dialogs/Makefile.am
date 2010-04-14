INCLUDES=-I$(top_builddir)/headers \
  -I.. -I$(srcdir)/.. \
  @all_includes@

AM_CFLAGS=-DBUILDING_AQBANKING @visibility_cflags@

noinst_LTLIBRARIES=libabdialogs.la

iheaderdir=${includedir}/aqbanking
iheader_HEADERS=\
  dlg_importer.h \
  dlg_selectbankinfo.h \
  dlg_setup.h \
  dlg_newaccount_be.h \
  dlg_newuser_be.h \
  dlg_selectbackend.h

noinst_HEADERS=\
  dlg_importer_p.h \
  dlg_selectbankinfo_p.h \
  dlg_setup_p.h \
  dlg_newaccount_p.h \
  dlg_newuser_p.h \
  dlg_selectbackend_p.h

libabdialogs_la_SOURCES=\
  dlg_importer.c \
  dlg_selectbankinfo.c \
  dlg_setup.c \
  dlg_newaccount.c \
  dlg_newuser.c \
  dlg_selectbackend.c


dlgdatadir=$(aqbanking_pkgdatadir)/dialogs
dlgdata_DATA=\
  dlg_importer.dlg \
  dlg_selectbankinfo.dlg \
  dlg_setup.dlg \
  dlg_selectbackend.dlg



EXTRA_DIST=$(typefiles) $(dlgdata_DATA)


sources:
	for f in $(libabdialogs_la_SOURCES); do \
	  echo $(subdir)/$$f >>$(top_srcdir)/i18nsources; \
	done
	for d in $(SUBDIRS); do \
	  $(MAKE) -C $$d sources; \
	done
