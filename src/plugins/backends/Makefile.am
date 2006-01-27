SUBDIRS = @aqbanking_backends@ aqnone
DIST_SUBDIRS = aqhbci aqdtaus aqgeldkarte aqofxconnect aqyellownet aqnone

sources:
	for d in $(SUBDIRS); do \
	  make -C $$d sources; \
	done

built_sources:
	for d in $(SUBDIRS); do \
	  make -C $$d built_sources; \
	done ;
