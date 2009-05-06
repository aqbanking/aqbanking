DIST_SUBDIRS=aqbanking-cli

if ENABLE_CLI
SUBDIRS=aqbanking-cli
endif


sources:
	for d in $(SUBDIRS); do \
	  $(MAKE) -C $$d sources; \
	done

built_sources:
	for d in $(SUBDIRS); do \
	  $(MAKE) -C $$d built_sources; \
	done ;
