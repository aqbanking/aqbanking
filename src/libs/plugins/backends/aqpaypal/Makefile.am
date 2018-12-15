SUBDIRS=control

INCLUDES=-I$(aqbanking_symlinkdir) $(gwenhywfar_includes) -I$(top_srcdir)/src/libs/aqbanking

AM_CFLAGS=@visibility_cflags@

DEFS += -DBUILDING_AQPAYPAL

#iheaderdir=${includedir}/aqpaypal
#iheader_HEADERS=

plugindir= $(aqbanking_plugindir)/providers
plugin_DATA=aqpaypal.xml


noinst_HEADERS=\
 aqpaypal.h \
 provider.h \
 user.h \
 aqpaypal_l.h \
 provider_l.h provider_p.h \
 user_p.h user_l.h \
 dlg_newuser_p.h dlg_newuser_l.h \
 dlg_edituser_p.h dlg_edituser_l.h \
 dlg_editsecret_p.h dlg_editsecret_l.h


# library
noinst_LTLIBRARIES=libaqpaypal.la
libaqpaypal_la_LIBADD=\
 control/libpaypalcontrol.la

libaqpaypal_la_SOURCES=\
  user.c \
  provider.c \
  dlg_newuser.c \
  dlg_edituser.c \
  dlg_editsecret.c


dlgdatadir=$(aqbanking_pkgdatadir)/backends/aqpaypal/dialogs
dlgdata_DATA=\
  dlg_newuser.dlg \
  dlg_edituser.dlg \
  dlg_editsecret.dlg


sources:
	for d in $(SUBDIRS); do \
	  $(MAKE) -C $$d sources; \
	done
	for f in $(dlgdata_DATA); do \
	  echo $(subdir)/$$f >>$(top_srcdir)/dlgsources; \
	done
	for f in $(plugin_DATA); do \
	  echo $(subdir)/$$f >>$(top_srcdir)/pdsources; \
	done
	for f in $(libaqpaypal_la_SOURCES); do \
	  echo $(subdir)/$$f >>$(top_srcdir)/i18nsources; \
	done

EXTRA_DIST=aqpaypal.xml.in $(dlgdata_DATA) $(dlgimage_DATA) \
  provider_accspec.c \
  provider_credentials.c \
  provider_dialogs.c \
  provider_getbalance.c \
  provider_getstm.c \
  provider_sendcmd.c \
  provider_update.c


built_sources:


cppcheck:
	for f in $(libaqpaypal_la_SOURCES); do \
	  cppcheck --force $$f ; \
	done
	for d in $(SUBDIRS); do \
	  $(MAKE) -C $$d cppcheck; \
	done



