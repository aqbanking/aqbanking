DIST_SUBDIRS=generic at ca ch de us
SUBDIRS=generic $(aqbanking_plugins_bankinfo_dirs)


sources:
	for d in $(SUBDIRS); do \
	  $(MAKE) -C $$d sources; \
	done

built_sources:
