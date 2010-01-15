SUBDIRS=@qt3_ui@ @qt4_ui@
DIST_SUBDIRS = qt3 qt4

sources:
	for d in $(SUBDIRS); do \
	  $(MAKE) -C $$d sources; \
	done

qt4-port:
	for d in $(SUBDIRS); do \
	  $(MAKE) -C $$d qt4-port; \
	done

built_sources:
	list='$(SUBDIRS)'; for d in $$list; do \
	  $(MAKE) -C $$d built_sources; \
	done ;
