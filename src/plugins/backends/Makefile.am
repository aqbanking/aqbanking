SUBDIRS = $(aqbanking_backends) aqnone
DIST_SUBDIRS = aqhbci aqdtaus aqgeldkarte aqofxconnect aqyellownet aqnone

sources:
	for d in $(DIST_SUBDIRS); do \
	  $(MAKE) -C $$d sources; \
	done

built_sources:
	for d in $(DIST_SUBDIRS); do \
	  $(MAKE) -C $$d built_sources; \
	done ;
