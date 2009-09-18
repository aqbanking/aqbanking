SUBDIRS=$(aqbanking_plugins_parsers_dirs)
DIST_SUBDIRS=dtaus swift


sources:
	for d in $(SUBDIRS); do \
	  $(MAKE) -C $$d sources; \
	done

built_sources:
