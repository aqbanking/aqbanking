SUBDIRS=$(aqbanking_plugins_imexporters_dirs)
DIST_SUBDIRS=qif dtaus ofx swift csv openhbci1 eri2 yellownet xmldb


noinst_LTLIBRARIES=libimexporters.la
libimexporters_la_SOURCES=dummy.c
libimexporters_la_LIBADD=$(aqbanking_plugins_imexporters_libs)


sources:
	for d in $(SUBDIRS); do \
	  $(MAKE) -C $$d sources; \
	done

built_sources:
