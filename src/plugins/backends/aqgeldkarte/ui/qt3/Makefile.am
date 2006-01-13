SUBDIRS=libs

sources:
	for d in $(SUBDIRS); do \
	  make -C $$d sources; \
	done

qt4-port:
	for d in $(SUBDIRS); do \
	  make -C $$d qt4-port; \
	done

built_sources:
	for d in $(SUBDIRS); do \
	  make -C $$d built_sources; \
	done ;
