SUBDIRS=$(aqbanking_plugins_imexporters_dirs)
DIST_SUBDIRS=qif ofx swift csv openhbci1 eri2 yellownet xmldb sepa ctxfile q43 camt


noinst_LTLIBRARIES=libabimexporters.la
libabimexporters_la_SOURCES=dummy.c
libabimexporters_la_LIBADD=@aqbanking_plugins_imexporters_libs@


sources:
	for d in $(SUBDIRS); do \
	  $(MAKE) -C $$d sources; \
	done

built_sources:


cppcheck:
	for d in $(SUBDIRS); do \
	  $(MAKE) -C $$d cppcheck; \
	done



