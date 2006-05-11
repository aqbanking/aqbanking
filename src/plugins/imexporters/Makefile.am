SUBDIRS=qif dbio dtaus ofx swift csv openhbci1 eri2 eri yellownet

sources:
	for d in $(SUBDIRS); do \
	  $(MAKE) -C $$d sources; \
	done

built_sources:
