INCLUDES=-I$(top_srcdir) -I$(top_srcdir)/headers @all_includes@

DEFS += -DBUILDING_AQNONE

AM_CFLAGS=@visibility_cflags@

EXTRA_DIST=aqnone.xml.in


plugindir=@aqbanking_plugindir@/providers
plugin_LTLIBRARIES=aqnone.la
if IS_WINDOWS
plugin_DATA=aqnone.xml aqnone.dll
else
plugin_DATA=aqnone.xml
endif


lib_LTLIBRARIES=libaqnone.la 

noinst_HEADERS=provider_l.h provider_p.h

libaqnone_la_SOURCES=provider.c
aqnone_la_SOURCES=aqnone-plugin.c

libaqnone_la_LIBADD=@gwenhywfar_libs@ @aqbanking_libs@

aqnone_la_LIBADD=libaqnone.la @gwenhywfar_libs@ @aqbanking_libs@
aqnone_la_LDFLAGS=-module -version-info 0:0:0


sources:
	for f in $(libaqnone_la_SOURCES); do \
	  echo $(subdir)/$$f >>$(top_srcdir)/i18nsources; \
	done
	for d in $(SUBDIRS); do \
	  make -C $$d sources; \
	done


