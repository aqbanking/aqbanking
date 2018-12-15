SUBDIRS = aqebics-tool

sources:

built_sources:


cppcheck:
	for d in $(SUBDIRS); do \
	  $(MAKE) -C $$d cppcheck; \
	done

