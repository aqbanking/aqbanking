INCLUDES=-I$(top_builddir)/headers $(LIBOFX_INCLUDES) $(all_includes)

localedir=$(datadir)/locale
DEFS += -DLOCALEDIR=\"$(localedir)\"

EXTRA_DIST=aqofxconnect.xml.in

plugindir = $(aqbanking_plugindir)/providers
plugin_LTLIBRARIES = aqofxconnect.la
if IS_WINDOWS
plugin_DATA = aqofxconnect.xml aqofxconnect.dll
else
plugin_DATA = aqofxconnect.xml
endif


lib_LTLIBRARIES=libaqofxconnect.la 

noinst_HEADERS=\
 account_p.h \
 aqofxconnect_l.h \
 provider_p.h \
 bank_p.h \
 context_l.h \
 context_p.h \
 user_p.h \
 queues_l.h \
 queues_p.h


iheaderdir=$(includedir)/aqofxconnect
iheader_HEADERS=\
 account.h \
 aqofxconnect.h \
 provider.h \
 bank.h \
 user.h

libaqofxconnect_la_SOURCES= \
 account.c \
 provider.c \
 bank.c \
 context.c \
 queues.c \
 user.c

aqofxconnect_la_SOURCES= \
 aqofxconnect-plugin.c

libaqofxconnect_la_LIBADD= \
  $(aqbanking_internal_libs) \
  $(LIBOFX_LIBS) \
  $(gwenhywfar_libs)
libaqofxconnect_la_LDFLAGS = -no-undefined \
  -version-info $(AQOFXCONNECT_SO_CURRENT):$(AQOFXCONNECT_SO_REVISION):$(AQOFXCONNECT_SO_AGE)

aqofxconnect_la_LIBADD = \
  libaqofxconnect.la \
  $(aqbanking_internal_libs) \
  $(LIBOFX_LIBS) \
  $(gwenhywfar_libs)
aqofxconnect_la_LDFLAGS = -no-undefined -module \
  -version-info $(AQOFXCONNECT_SO_CURRENT):$(AQOFXCONNECT_SO_REVISION):$(AQOFXCONNECT_SO_AGE)


sources:
	for f in $(libaqofxconnect_la_SOURCES) $(aqofxconnect_la_SOURCES); do \
	  echo $(subdir)/$$f >>$(top_srcdir)/i18nsources; \
	done
	for d in $(SUBDIRS); do \
	  make -C $$d sources; \
	done

built_sources:
