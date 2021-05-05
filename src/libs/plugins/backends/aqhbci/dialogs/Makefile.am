AM_CPPFLAGS = -I$(top_srcdir)/src/libs -I$(top_builddir)/src/libs $(gwenhywfar_includes) \
  -I$(srcdir)/../.. \
  -I.. -I$(srcdir)/../banking \
  -I.. -I$(srcdir)/../msglayer \
  -I.. -I$(srcdir)/../joblayer \
  -I$(srcdir)/../tan \
  -I.. -I$(srcdir)/.. \
  -I$(top_srcdir)/src/libs/aqbanking $(gwenhywfar_includes)

AM_CFLAGS=-DBUILDING_AQBANKING @visibility_cflags@

noinst_LTLIBRARIES=libaqhbci_dlgs.la

#iheaderdir=@aqbanking_headerdir_am@/aqhbci
#iheader_HEADERS=

noinst_HEADERS=\
  dlg_edituserddv_l.h dlg_edituserddv_p.h \
  dlg_edituserpintan_l.h dlg_edituserpintan_p.h \
  dlg_edituserrdh_l.h dlg_edituserrdh_p.h \
  dlg_pintan_l.h dlg_pintan_p.h \
  dlg_pintan_special_l.h dlg_pintan_special_p.h \
  dlg_newuser_l.h dlg_newuser_p.h \
  dlg_ddvcard_l.h dlg_ddvcard_p.h \
  dlg_zkacard_l.h dlg_zkacard_p.h \
  dlg_ddvcard_special_l.h dlg_ddvcard_special_p.h \
  dlg_newkeyfile_l.h dlg_newkeyfile_p.h \
  dlg_rdh_special_l.h dlg_rdh_special_p.h \
  dlg_importkeyfile_l.h dlg_importkeyfile_p.h \
  dlg_choose_usertype_l.h dlg_choose_usertype_p.h \
  dlg_editaccount_l.h dlg_editaccount_p.h \
  dlg_pintan_tanmode_l.h dlg_pintan_tanmode_p.h

libaqhbci_dlgs_la_SOURCES=\
  dlg_edituserddv.c \
  dlg_ddvcard_special.c \
  dlg_edituserpintan.c \
  dlg_edituserrdh.c \
  dlg_pintan.c \
  dlg_pintan_special.c \
  dlg_ddvcard.c \
  dlg_zkacard.c \
  dlg_newkeyfile.c \
  dlg_rdh_special.c \
  dlg_importkeyfile.c \
  dlg_newuser.c \
  dlg_choose_usertype.c \
  dlg_editaccount.c \
  dlg_pintan_tanmode.c


dlgdatadir=$(aqbanking_pkgdatadir)/backends/aqhbci/dialogs
dlgdata_DATA=\
  dlg_edituserddv.dlg \
  dlg_edituserpintan.dlg \
  dlg_edituserrdh.dlg \
  dlg_ddvcard.dlg \
  dlg_zkacard.dlg \
  dlg_ddvcard_special.dlg \
  dlg_pintan.dlg \
  dlg_pintan_special.dlg \
  dlg_newkeyfile.dlg \
  dlg_rdh_special.dlg \
  dlg_importkeyfile.dlg \
  dlg_newuser.dlg \
  dlg_choose_usertype.dlg \
  dlg_editaccount.dlg \
  dlg_pintan_tanmode.dlg


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


typefiles:
	for d in $(SUBDIRS); do \
	  $(MAKE) -C $$d typefiles; \
	done

typedefs:
	for d in $(SUBDIRS); do \
	  $(MAKE) -C $$d typedefs; \
	done



cppcheck:
	for f in $(libaqhbci_dlgs_la_SOURCES); do \
	  cppcheck --force $$f ; \
	done
	for d in $(SUBDIRS); do \
	  $(MAKE) -C $$d cppcheck; \
	done



