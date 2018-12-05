DIST_SUBDIRS=generic de
SUBDIRS=generic $(aqbanking_plugins_bankinfo_dirs)


sources:
	for d in $(SUBDIRS); do \
	  $(MAKE) -C $$d sources; \
	done

built_sources:
