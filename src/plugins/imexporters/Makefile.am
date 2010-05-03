SUBDIRS=$(aqbanking_plugins_imexporters_dirs)
DIST_SUBDIRS=qif dtaus ofx swift csv openhbci1 eri2 yellownet xmldb sepa ctxfile q43


sources:
	for d in $(SUBDIRS); do \
	  $(MAKE) -C $$d sources; \
	done

built_sources:
