DIST_SUBDIRS=generic at ca ch de us
SUBDIRS=generic $(aqbanking_plugins_bankinfo_dirs)


noinst_LTLIBRARIES=libbankinfo.la
libbankinfo_la_SOURCES=dummy.c
libbankinfo_la_LIBADD=$(aqbanking_plugins_bankinfo_libs) \
  generic/libbankinfo_generic.la


sources:
	for d in $(SUBDIRS); do \
	  $(MAKE) -C $$d sources; \
	done

built_sources:
