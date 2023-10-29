
AM_CPPFLAGS = -I$(top_srcdir)/src/libs -I$(top_builddir)/src/libs $(gwenhywfar_includes) -I$(srcdir)/..

AM_CFLAGS=-DBUILDING_AQBANKING @visibility_cflags@

EXTRA_DIST=aqgivve.xml.in

plugindir=@aqbanking_plugindir@/providers
plugin_DATA=aqgivve.xml

noinst_HEADERS=\
	cardsdialog.h \
	cardsdialog_p.h \
	merchant.h \
	merchant_p.h \
	meta.h \
	meta_p.h \
	provider_p.h \
	provider.h \
	provider_request.h \
	provider_request_p.h \	
	userdialog.h \
	userdialog_p.h \
	voucher.h \
	voucher_p.h \
	voucherlist.h \
	voucherlist_p.h \
	voucherowner.h \
	voucherowner_p.h

# static plugin library
noinst_LTLIBRARIES=libaqgivve.la 
libaqgivve_la_SOURCES=\
	cardsdialog.c \
	merchant.c \
	meta.c \
	provider.c \
	provider_request.c \
	userdialog.c \
	voucher.c \
	voucherlist.c \
	voucherowner.c 

dlgdatadir=$(aqbanking_pkgdatadir)/backends/aqgivve/dialogs
dlgdata_DATA=\
	dlg_edituser.dlg \
	dlg_cardselect.dlg

EXTRA_DIST=$(dlgdata_DATA)
  
sources:
	for f in $(libaqgivve_la_SOURCES); do \
	  echo $(subdir)/$$f >>$(top_srcdir)/i18nsources; \
	done
	for f in $(plugin_DATA); do \
	  echo $(subdir)/$$f >>$(top_srcdir)/pdsources; \
	done
	for d in $(SUBDIRS); do \
	  $(MAKE) -C $$d sources; \
	done

built_sources:


cppcheck:
	for f in $(libaqgivve_la_SOURCES); do \
	  cppcheck --force $$f ; \
	done
	for d in $(SUBDIRS); do \
	  $(MAKE) -C $$d cppcheck; \
	done


typefiles:

typedefs:

