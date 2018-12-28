SUBDIRS=libofxhome

INCLUDES=-I$(aqbanking_symlinkdir) -I$(top_srcdir)/src/libs/aqbanking $(gwenhywfar_includes)

AM_CFLAGS=@visibility_cflags@

localedir=$(datadir)/locale
DEFS += -DLOCALEDIR=\"$(localedir)\" -DBUILDING_AQOFXCONNECT

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
  libofxhome/libofxhome.la


EXTRA_DIST= $(dlgdata_DATA) $(dlgimage_DATA) \
  aqofxconnect.xml.in \
  provider_accspec.c \
  provider_cmd_accinfo.c \
  provider_cmd_stm.c \
  provider_network.c \
  provider_request.c \
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


