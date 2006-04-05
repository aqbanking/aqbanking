INCLUDES=-I$(top_builddir)/headers @all_includes@

localedir=$(datadir)/locale
DEFS += -DLOCALEDIR=\"$(localedir)\"

EXTRA_DIST=aqgeldkarte.xml.in


plugindir=@aqbanking_plugindir@/providers
plugin_LTLIBRARIES=aqgeldkarte.la
if IS_WINDOWS
plugin_DATA = aqgeldkarte.xml 
else
plugin_DATA=aqgeldkarte.xml
endif


lib_LTLIBRARIES=libaqgeldkarte.la 

noinst_HEADERS=\
 account_l.h \
 account_p.h \
 aqgeldkarte_l.h \
 card_l.h \
 card_p.h \
 provider_p.h


iheaderdir=${includedir}/aqgeldkarte
iheader_HEADERS=\
 account.h \
 aqgeldkarte.h \
 provider.h

libaqgeldkarte_la_SOURCES= \
 account.c \
 card.c \
 provider.c

aqgeldkarte_la_SOURCES= \
 aqgeldkarte-plugin.c

libaqgeldkarte_la_LIBADD= $(aqbanking_internal_libs) \
  $(chipcard_client_libs) \
  $(gwenhywfar_libs)
aqgeldkarte_la_LIBADD= \
  libaqgeldkarte.la \
  $(aqbanking_internal_libs) \
  $(chipcard_client_libs) \
  $(gwenhywfar_libs)
libaqgeldkarte_la_LDFLAGS = -no-undefined \
  -version-info @AQGELDKARTE_SO_CURRENT@:@AQGELDKARTE_SO_REVISION@:@AQGELDKARTE_SO_AGE@
aqgeldkarte_la_LDFLAGS = -no-undefined -module \
  -version-info @AQGELDKARTE_SO_CURRENT@:@AQGELDKARTE_SO_REVISION@:@AQGELDKARTE_SO_AGE@

# Add the linker flags for windows resource file
libaqgeldkarte_la_DEPENDENCIES = $(aqbanking_internal_libs)
aqgeldkarte_la_DEPENDENCIES = libaqgeldkarte.la $(aqbanking_internal_libs)
# if IS_WINDOWS
if IS_WINDOWS
libaqgeldkarte_la_DEPENDENCIES += ressource.o
aqgeldkarte_la_DEPENDENCIES += ressource.o
libaqgeldkarte_la_LDFLAGS += -Wl,ressource.o
aqgeldkarte_la_LDFLAGS += -Wl,ressource.o
ressource.o: ressource.rc
	$(RC) -o $@ $<
endif
# IS_WINDOWS
# End of linker flags for windows resource file


sources:
	for f in $(libaqgeldkarte_la_SOURCES); do \
	  echo $(subdir)/$$f >>$(top_srcdir)/i18nsources; \
	done
	for d in $(SUBDIRS); do \
	  $(MAKE) -C $$d sources; \
	done

built_sources:
	for d in $(SUBDIRS); do \
	  $(MAKE) -C $$d built_sources; \
	done ;

