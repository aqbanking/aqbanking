INCLUDES=-I$(aqbanking_symlinkdir) $(gwenhywfar_includes)

localedir=$(datadir)/locale
DEFS += -DLOCALEDIR=\"$(localedir)\" -DBUILDING_AQOFXCONNECT

EXTRA_DIST=\
  aqofxconnect.xml.in \
  network.c \
  request.c \
  r_statements.c \
  r_accountinfo.c


plugindir = $(aqbanking_plugindir)/providers
plugin_DATA = aqofxconnect.xml


noinst_HEADERS=\
 account_p.h \
 aqofxconnect_l.h \
 provider_p.h \
 user_p.h \
 queues_l.h \
 queues_p.h


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
 user.c
libaqofxconnect_la_LIBADD= \
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


sources:
	for f in $(libaqofxconnect_la_SOURCES) $(aqofxconnect_la_SOURCES); do \
	  echo $(subdir)/$$f >>$(top_srcdir)/i18nsources; \
	done
	for d in $(SUBDIRS); do \
	  $(MAKE) -C $$d sources; \
	done

built_sources:
