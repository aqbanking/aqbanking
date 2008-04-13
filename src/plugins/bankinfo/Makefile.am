DIST_SUBDIRS=generic at ca ch de us
SUBDIRS=generic $(aqbanking_plugins_bankinfo_dirs)


noinst_LTLIBRARIES=libbankinfo.la
libbankinfo_la_SOURCES=dummy.c


sources:
	for d in $(SUBDIRS); do \
	  $(MAKE) -C $$d sources; \
	done

built_sources:
