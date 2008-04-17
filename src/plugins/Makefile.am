SUBDIRS=parsers imexporters bankinfo backends

INCLUDES = -I$(top_builddir)/headers $(gwenhywfar_includes)


noinst_LTLIBRARIES=libaqb_plugins.la
libaqb_plugins_la_SOURCES=aqbanking_plugins.c
libaqb_plugins_la_LIBADD=\
  backends/libbackends.la \
  bankinfo/libbankinfo.la \
  imexporters/libimexporters.la \
  parsers/libparsers.la
noinst_HEADERS=aqbanking_plugins.h

sources:
	for d in $(SUBDIRS); do \
	  $(MAKE) -C $$d sources; \
	done


built_sources:
	for d in $(SUBDIRS); do \
	  $(MAKE) -C $$d built_sources; \
	done ;
