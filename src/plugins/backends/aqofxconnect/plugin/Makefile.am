INCLUDES=-I$(top_builddir)/headers $(gwenhywfar_includes)

localedir=$(datadir)/locale
DEFS += -DLOCALEDIR=\"$(localedir)\" -DBUILDING_AQOFXCONNECT

EXTRA_DIST=\
  aqofxconnect.xml.in \
  network.c \
  context1.c \
  context2.c \
  request.c \
  r_statements.c \
  r_accountinfo.c


plugindir = $(aqbanking_plugindir)/providers
plugin_LTLIBRARIES = aqofxconnect.la
if IS_WINDOWS
plugin_DATA = aqofxconnect.xml 
else
plugin_DATA = aqofxconnect.xml
endif


lib_LTLIBRARIES=libaqofxconnect.la 

noinst_HEADERS=\
 account_p.h \
 aqofxconnect_l.h \
 provider_p.h \
 user_p.h \
 queues_l.h \
 queues_p.h


iheaderdir=$(includedir)/aqofxconnect
iheader_HEADERS=\
 account.h \
 aqofxconnect.h \
 provider.h \
 user.h

libaqofxconnect_la_SOURCES= \
 account.c \
 provider.c \
 queues.c \
 user.c

aqofxconnect_la_SOURCES= \
 aqofxconnect-plugin.c

libaqofxconnect_la_LIBADD= \
  $(aqbanking_internal_libs) \
  $(gwenhywfar_libs)
libaqofxconnect_la_LDFLAGS = -no-undefined \
  -version-info $(AQOFXCONNECT_SO_CURRENT):$(AQOFXCONNECT_SO_REVISION):$(AQOFXCONNECT_SO_AGE)

aqofxconnect_la_LIBADD = \
  libaqofxconnect.la \
  $(aqbanking_internal_libs) \
  $(gwenhywfar_libs)
aqofxconnect_la_LDFLAGS = -no-undefined -module -avoid-version

# Add the linker flags for windows resource file
libaqofxconnect_la_DEPENDENCIES = $(aqbanking_internal_libs)
aqofxconnect_la_DEPENDENCIES = libaqofxconnect.la $(aqbanking_internal_libs)
# if IS_WINDOWS
if IS_WINDOWS
libaqofxconnect_la_DEPENDENCIES += ressource.o
aqofxconnect_la_DEPENDENCIES += ressource.o
libaqofxconnect_la_LDFLAGS += -Wl,ressource.o
aqofxconnect_la_LDFLAGS += -Wl,ressource.o
ressource.o: ressource.rc
	$(RC) -o $@ $<
endif
# IS_WINDOWS
# End of linker flags for windows resource file


sources:
	for f in $(libaqofxconnect_la_SOURCES) $(aqofxconnect_la_SOURCES); do \
	  echo $(subdir)/$$f >>$(top_srcdir)/i18nsources; \
	done
	for d in $(SUBDIRS); do \
	  $(MAKE) -C $$d sources; \
	done

built_sources:
