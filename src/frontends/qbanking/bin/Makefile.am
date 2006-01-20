SUBDIRS=wizard helpbrowser

sources:
	for d in $(SUBDIRS); do \
	  make -C $$d sources; \
	done ;

# This section is for a qt4 environment
qt4-port:
	for d in $(SUBDIRS); do \
	  make -C $$d qt4-port; \
	done

built_sources: $(BUILT_SOURCES)
	for d in $(SUBDIRS); do \
	  make -C $$d built_sources; \
	done ;

