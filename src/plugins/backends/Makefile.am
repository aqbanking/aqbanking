SUBDIRS = $(aqbanking_backends)
DIST_SUBDIRS = aqhbci aqdtaus aqgeldkarte aqofxconnect

sources:
	for d in $(SUBDIRS); do \
	  make -C $$d sources; \
	done
