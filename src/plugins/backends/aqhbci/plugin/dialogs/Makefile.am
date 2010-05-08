INCLUDES=-I$(aqbanking_symlinkdir) \
  -I.. -I$(srcdir)/.. \
  -I$(top_srcdir)/src/libs/aqbanking $(gwenhywfar_includes)

AM_CFLAGS=-DBUILDING_AQHBCI @visibility_cflags@

noinst_LTLIBRARIES=libaqhbci_dlgs.la

iheaderdir=@aqbanking_headerdir_am@/aqhbci
iheader_HEADERS=\
  dlg_pintan.h \
  dlg_pintan_special.h \
  dlg_newuser.h \
  dlg_ddvcard.h \
  dlg_ddvcard_special.h

noinst_HEADERS=\
  dlg_pintan_p.h \
  dlg_pintan_special_p.h \
  dlg_newuser_p.h \
  dlg_ddvcard_p.h \
  dlg_ddvcard_special_p.h

libaqhbci_dlgs_la_SOURCES=\
  dlg_pintan.c \
  dlg_pintan_special.c \
  dlg_newuser.c \
  dlg_ddvcard.c \
  dlg_ddvcard_special.c


dlgdatadir=$(aqbanking_pkgdatadir)/backends/aqhbci/dialogs
dlgdata_DATA=\
  dlg_ddvcard.dlg \
  dlg_ddvcard_special.dlg \
  dlg_pintan.dlg \
  dlg_pintan_special.dlg \
  dlg_newuser.dlg

dlgimagedir=$(aqbanking_pkgdatadir)/backends/aqhbci/dialogs
dlgimage_DATA=\
  chipcard.png \
  keyfile.png \
  pintan.png


EXTRA_DIST=$(typefiles) $(dlgdata_DATA) $(dlgimage_DATA)


sources:
	for f in $(libaqhbci_dlgs_la_SOURCES); do \
	  echo $(subdir)/$$f >>$(top_srcdir)/i18nsources; \
	done
	for f in $(dlgdata_DATA); do \
	  echo $(subdir)/$$f >>$(top_srcdir)/dlgsources; \
	done
	for d in $(SUBDIRS); do \
	  $(MAKE) -C $$d sources; \
	done
