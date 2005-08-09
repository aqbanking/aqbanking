SUBDIRS=libs qt3_wizard po

SUBPACKAGE=aqdtaus-qt-tools


qt4-port:
	for d in $(SUBDIRS); do \
	  make -C $$d qt4-port; \
	done


sources:
	for d in $(SUBDIRS); do \
	  make -C $$d sources; \
	done ;

