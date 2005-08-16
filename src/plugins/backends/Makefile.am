SUBDIRS = $(aqbanking_backends)
DIST_SUBDIRS = aqhbci aqdtaus aqgeldkarte aqofxconnect

sources:
	for d in $(SUBDIRS); do \
	  make -C $$d sources; \
	done

built_sources:
	for d in $(SUBDIRS); do \
	  make -C $$d built_sources; \
	done ;
