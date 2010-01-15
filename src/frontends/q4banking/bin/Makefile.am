SUBDIRS=wizard helpbrowser

sources:
	for d in $(SUBDIRS); do \
	  $(MAKE) -C $$d sources; \
	done ;

built_sources: $(BUILT_SOURCES)
	for d in $(SUBDIRS); do \
	  $(MAKE) -C $$d built_sources; \
	done ;

