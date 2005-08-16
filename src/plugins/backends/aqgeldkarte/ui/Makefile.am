SUBDIRS=@aqgeldkarte_ui@
DIST_SUBDIRS = qt3

sources:
	for d in $(SUBDIRS); do \
	  make -C $$d sources; \
	done

qt4-port:
	for d in $(SUBDIRS); do \
	  make -C $$d qt4-port; \
	done

built_sources:
	list='$(SUBDIRS)'; for d in $$list; do \
	  make -C $$d built_sources; \
	done ;
