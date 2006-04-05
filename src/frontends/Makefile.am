SUBDIRS = $(aqbanking_frontends)
DIST_SUBDIRS = cbanking g2banking qbanking kbanking

sources:
	for d in $(DIST_SUBDIRS); do \
	  $(MAKE) -C $$d sources; \
	done

built_sources:
	for d in $(DIST_SUBDIRS); do \
	  $(MAKE) -C $$d built_sources; \
	done ;
