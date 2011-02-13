SUBDIRS=libofxhome

INCLUDES=-I$(aqbanking_symlinkdir) -I$(top_srcdir)/src/libs/aqbanking $(gwenhywfar_includes)

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
  queues_l.h \
  queues_p.h \
  dlg_edituser_p.h dlg_edituser_l.h \
  dlg_newuser_p.h dlg_newuser_l.h \
  dlg_ofx_special_p.h dlg_ofx_special_l.h


iheaderdir=@aqbanking_headerdir_am@/aqofxconnect
iheader_HEADERS=\
  account.h \
  aqofxconnect.h \
  provider.h \
  user.h

# library
lib_LTLIBRARIES=libaqofxconnect.la 
libaqofxconnect_la_SOURCES= \
  account.c \
  provider.c \
  queues.c \
  user.c \
  dlg_edituser.c \
  dlg_newuser.c \
  dlg_ofx_special.c

libaqofxconnect_la_LIBADD= \
  libofxhome/libofxhome.la \
  $(aqbanking_internal_libs) \
  $(gwenhywfar_libs)
libaqofxconnect_la_LDFLAGS = -no-undefined \
  -version-info $(AQOFXCONNECT_SO_CURRENT):$(AQOFXCONNECT_SO_REVISION):$(AQOFXCONNECT_SO_AGE)


# static plugin library
noinst_LTLIBRARIES=libaqofxconnectplugin.la 
libaqofxconnectplugin_la_SOURCES= \
  aqofxconnect-plugin.c
libaqofxconnectplugin_la_LIBADD= \
  libaqofxconnect.la \
  $(aqbanking_internal_libs) \
  $(gwenhywfar_libs)
libaqofxconnectplugin_la_LDFLAGS = -no-undefined


# dynamic plugin
plugin_LTLIBRARIES = aqofxconnect.la
aqofxconnect_la_SOURCES=dummy.c
aqofxconnect_la_LIBADD = \
  libaqofxconnectplugin.la \
  $(aqbanking_internal_libs) \
  $(gwenhywfar_libs)
aqofxconnect_la_LDFLAGS = -no-undefined -module -avoid-version


EXTRA_DIST= $(dlgdata_DATA) $(dlgimage_DATA) \
  aqofxconnect.xml.in \
  network.c \
  request.c \
  r_statements.c \
  r_accountinfo.c


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



