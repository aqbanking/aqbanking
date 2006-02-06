INCLUDES = \
  -I$(top_srcdir) \
  -I$(top_builddir)/headers \
  $(all_includes)

libfiles=\
 libaqyellownet.la \
 libaqyellownet.so

EXTRA_DIST=$(libfiles) aqyellownet.xml.in COPYING

iheaderdir=${includedir}/aqyellownet
iheader_HEADERS=\
 account.h \
 aqyellownet.h \
 provider.h \
 user.h

plugindir=@aqbanking_plugindir@/providers
plugin_LTLIBRARIES=aqyellownet.la
plugin_DATA=aqyellownet.xml

aqyellownet_la_SOURCES= \
 aqyellownet-plugin.c

aqyellownet_la_LIBADD= \
  $(srcdir)/libaqyellownet.la \
  $(aqbanking_internal_libs) \
  $(gwenhywfar_libs)
aqyellownet_la_LDFLAGS=-module

install-exec-local:
	$(mkinstalldirs) $(DESTDIR)$(libdir)
	cp $(libfiles) $(DESTDIR)$(libdir)

sources:

built_sources: $(BUILT_SOURCES)
