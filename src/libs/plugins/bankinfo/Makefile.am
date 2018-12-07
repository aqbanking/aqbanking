DIST_SUBDIRS=generic de
SUBDIRS=generic $(aqbanking_plugins_bankinfo_dirs)


noinst_LTLIBRARIES=libabbankinfo.la
libabbankinfo_la_SOURCES=dummy.c
libabbankinfo_la_LIBADD=@aqbanking_plugins_bankinfo_libs@


sources:
	for d in $(SUBDIRS); do \
	  $(MAKE) -C $$d sources; \
	done

built_sources:
