INCLUDES=-I$(aqbanking_symlinkdir) $(gwenhywfar_includes) -I$(top_srcdir)/src/libs/aqbanking

DEFS += -DBUILDING_AQPAYPAL

iheaderdir=${includedir}/aqpaypal
iheader_HEADERS=\
 aqpaypal.h \
 provider.h \
 user.h

plugindir= $(aqbanking_plugindir)/providers
plugin_DATA=aqpaypal.xml


noinst_HEADERS=\
 aqpaypal_l.h \
 provider_l.h provider_p.h \
 user_p.h user_l.h \
 dlg_newuser_p.h dlg_newuser_l.h \
 dlg_edituser_p.h dlg_edituser_l.h


# library
lib_LTLIBRARIES=libaqpaypal.la
libaqpaypal_la_LIBADD=$(gwenhywfar_libs) $(aqbanking_internal_libs)
libaqpaypal_la_SOURCES=\
  provider.c \
  user.c \
  dlg_newuser.c \
  dlg_edituser.c


# plugin
plugin_LTLIBRARIES=aqpaypal.la
aqpaypal_la_SOURCES=aqpaypal-plugin.c
aqpaypal_la_LDFLAGS=-module -avoid-version
aqpaypal_la_LIBADD=libaqpaypal.la $(gwenhywfar_libs) $(aqbanking_internal_libs)


dlgdatadir=$(aqbanking_pkgdatadir)/backends/aqpaypal/dialogs
dlgdata_DATA=\
  dlg_newuser.dlg \
  dlg_edituser.dlg


sources:
	for d in $(SUBDIRS); do \
	  $(MAKE) -C $$d sources; \
	done
	for f in $(libaqpaypal_la_SOURCES); do \
	  echo $(subdir)/$$f >>$(top_srcdir)/i18nsources; \
	done

EXTRA_DIST=aqpaypal.xml.in $(dlgdata_DATA) $(dlgimage_DATA)

built_sources:


