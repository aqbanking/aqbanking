INCLUDES=-I$(aqbanking_symlinkdir) $(gwenhywfar_includes)

EXTRA_DIST=aqpaypal.xml.in

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
 user_p.h user_l.h


# library
lib_LTLIBRARIES=libaqpaypal.la
libaqpaypal_la_LIBADD=$(gwenhywfar_libs) $(aqbanking_internal_libs)
libaqpaypal_la_SOURCES=\
  provider.c \
  user.c

# plugin
plugin_LTLIBRARIES=aqpaypal.la
aqpaypal_la_SOURCES=aqpaypal-plugin.c
aqpaypal_la_LDFLAGS=-module -avoid-version
aqpaypal_la_LIBADD=libaqpaypal.la $(gwenhywfar_libs) $(aqbanking_internal_libs)

sources:
	for d in $(SUBDIRS); do \
	  $(MAKE) -C $$d sources; \
	done
	for f in $(libaqpaypal_la_SOURCES); do \
	  echo $(subdir)/$$f >>$(top_srcdir)/i18nsources; \
	done


built_sources:
