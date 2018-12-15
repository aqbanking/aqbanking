SUBDIRS=libs tools test

sources:
	for d in $(SUBDIRS); do \
	  $(MAKE) -C $$d sources; \
	done

built_sources:
	for d in $(SUBDIRS); do \
	  $(MAKE) -C $$d built_sources; \
	done ;

cppcheck:
	for d in $(SUBDIRS); do \
	  $(MAKE) -C $$d cppcheck; \
	done


