SUBDIRS=$(aqbanking_plugins_parsers_dirs)
DIST_SUBDIRS=dtaus swift


noinst_LTLIBRARIES=libparsers.la
libparsers_la_SOURCES=dummy.c
libparsers_la_LIBADD=$(aqbanking_plugins_parsers_libs)


sources:
	for d in $(SUBDIRS); do \
	  $(MAKE) -C $$d sources; \
	done

built_sources:
