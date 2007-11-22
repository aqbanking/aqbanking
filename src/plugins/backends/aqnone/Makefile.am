INCLUDES = \
  -I$(top_srcdir) \
  -I$(top_builddir)/headers \
  $(all_includes)

DEFS += -DBUILDING_AQNONE

AM_CFLAGS=@visibility_cflags@

EXTRA_DIST=aqnone.xml.in


plugindir=@aqbanking_plugindir@/providers
plugin_LTLIBRARIES=aqnone.la
if IS_WINDOWS
plugin_DATA=aqnone.xml
else
plugin_DATA=aqnone.xml
endif


lib_LTLIBRARIES=libaqnone.la 

noinst_HEADERS=provider_l.h provider_p.h

libaqnone_la_SOURCES=provider.c
aqnone_la_SOURCES=aqnone-plugin.c

libaqnone_la_LIBADD = $(aqbanking_internal_libs) $(gwenhywfar_libs) 
libaqnone_la_LDFLAGS = -no-undefined -version-info @AQBANKING_SO_CURRENT@:@AQBANKING_SO_REVISION@:@AQBANKING_SO_AGE@

aqnone_la_LIBADD = libaqnone.la $(aqbanking_internal_libs) $(gwenhywfar_libs) 
aqnone_la_LDFLAGS = -no-undefined -module -avoid-version

# Add the linker flags for windows resource file
libaqnone_la_DEPENDENCIES = $(aqbanking_internal_libs)
aqnone_la_DEPENDENCIES = libaqnone.la $(aqbanking_internal_libs)
# if IS_WINDOWS
if IS_WINDOWS
libaqnone_la_DEPENDENCIES += ressource.o
aqnone_la_DEPENDENCIES += ressource.o
libaqnone_la_LDFLAGS += -Wl,ressource.o
aqnone_la_LDFLAGS += -Wl,ressource.o
ressource.o: ressource.rc
	$(RC) -o $@ $<
endif
# IS_WINDOWS
# End of linker flags for windows resource file

sources:
	for f in $(libaqnone_la_SOURCES); do \
	  echo $(subdir)/$$f >>$(top_srcdir)/i18nsources; \
	done
	for d in $(SUBDIRS); do \
	  $(MAKE) -C $$d sources; \
	done

built_sources:
