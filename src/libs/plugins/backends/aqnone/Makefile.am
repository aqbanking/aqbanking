INCLUDES = \
  -I$(top_srcdir) \
  -I$(aqbanking_symlinkdir) \
  $(all_includes)

DEFS += -DBUILDING_AQNONE

AM_CFLAGS=@visibility_cflags@

EXTRA_DIST=aqnone.xml.in

plugindir=@aqbanking_plugindir@/providers
plugin_DATA=aqnone.xml

noinst_HEADERS=provider_l.h provider_p.h

# static plugin library
noinst_LTLIBRARIES=libaqnone.la 
libaqnone_la_SOURCES=provider.c

sources:
	for f in $(libaqnone_la_SOURCES); do \
	  echo $(subdir)/$$f >>$(top_srcdir)/i18nsources; \
	done
	for f in $(plugin_DATA); do \
	  echo $(subdir)/$$f >>$(top_srcdir)/pdsources; \
	done
	for d in $(SUBDIRS); do \
	  $(MAKE) -C $$d sources; \
	done

built_sources:


cppcheck:
	for f in $(libaqnone_la_SOURCES); do \
	  cppcheck --force $$f ; \
	done
	for d in $(SUBDIRS); do \
	  $(MAKE) -C $$d cppcheck; \
	done


