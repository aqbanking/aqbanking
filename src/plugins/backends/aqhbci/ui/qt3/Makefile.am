SUBDIRS=libs qt3_debug

qt4-port:
	for d in $(SUBDIRS); do \
	  $(MAKE) -C $$d qt4-port; \
	done


sources:
	for d in $(SUBDIRS); do \
	  $(MAKE) -C $$d sources; \
	done ;

built_sources:
	for d in $(SUBDIRS); do \
	  $(MAKE) -C $$d built_sources; \
	done ;

