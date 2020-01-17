SUBDIRS=libofxhome v2 control

AM_CPPFLAGS = -I$(top_srcdir)/src/libs -I$(top_builddir)/src/libs $(gwenhywfar_includes) -I$(srcdir)/..

AM_CFLAGS=-DBUILDING_AQBANKING @visibility_cflags@

localedir=$(datadir)/locale
DEFS += -DLOCALEDIR=\"$(localedir)\"

plugindir = $(aqbanking_plugindir)/providers
plugin_DATA = aqofxconnect.xml


dlgdatadir=$(aqbanking_pkgdatadir)/backends/aqofxconnect/dialogs
dlgdata_DATA=\
  dlg_edituser.dlg \
  dlg_newuser.dlg \
  dlg_ofx_special.dlg


noinst_HEADERS=\
  account_p.h \
  aqofxconnect_l.h \
  provider_p.h \
  user_p.h \
  dlg_edituser_p.h dlg_edituser_l.h \
  dlg_newuser_p.h dlg_newuser_l.h \
  dlg_ofx_special_p.h dlg_ofx_special_l.h \
  account.h \
  aqofxconnect.h \
  provider.h \
  user.h


#iheaderdir=@aqbanking_headerdir_am@/aqofxconnect
#iheader_HEADERS=


# static plugin library
noinst_LTLIBRARIES=libaqofxconnect.la 
libaqofxconnect_la_SOURCES= \
  account.c \
  user.c \
  provider.c \
  dlg_edituser.c \
  dlg_newuser.c \
  dlg_ofx_special.c

libaqofxconnect_la_LIBADD= \
  libofxhome/libofxhome.la \
  v2/libofxdcv2.la \
  control/libofxdccontrol.la



EXTRA_DIST= $(dlgdata_DATA) $(dlgimage_DATA) \
  aqofxconnect.xml.in \
  provider_accspec.c \
  provider_sendcmd.c \
  provider_update.c


sources:
	for f in $(libaqofxconnect_la_SOURCES) $(aqofxconnect_la_SOURCES); do \
	  echo $(subdir)/$$f >>$(top_srcdir)/i18nsources; \
	done
	for f in $(dlgdata_DATA); do \
	  echo $(subdir)/$$f >>$(top_srcdir)/dlgsources; \
	done
	for f in $(plugin_DATA); do \
	  echo $(subdir)/$$f >>$(top_srcdir)/pdsources; \
	done
	for d in $(SUBDIRS); do \
	  $(MAKE) -C $$d sources; \
	done

built_sources:
	list='$(SUBDIRS)'; for d in $$list; do \
	  $(MAKE) -C $$d built_sources; \
	done ;


cppcheck:
	for f in $(libaqofxconnect_la_SOURCES); do \
	  cppcheck --force $$f ; \
	done
	for d in $(SUBDIRS); do \
	  $(MAKE) -C $$d cppcheck; \
	done


typefiles:
	for d in $(SUBDIRS); do \
	  $(MAKE) -C $$d typefiles; \
	done

typedefs:
	for d in $(SUBDIRS); do \
	  $(MAKE) -C $$d typedefs; \
	done

