SUBDIRS = $(aqbanking_frontends)
DIST_SUBDIRS = cbanking g2banking qbanking kbanking

sources:
	for d in $(SUBDIRS); do \
	  make -C $$d sources; \
	done
