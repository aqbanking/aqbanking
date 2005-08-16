SUBDIRS=parsers imexporters bankinfo backends

sources:
	for d in $(SUBDIRS); do \
	  make -C $$d sources; \
	done


built_sources:
	for d in $(SUBDIRS); do \
	  make -C $$d built_sources; \
	done ;
