SUBDIRS = $(aqbanking_plugins_backends_dirs)
DIST_SUBDIRS = aqfints aqhbci aqofxconnect aqnone aqebics aqpaypal aqgivve

noinst_LTLIBRARIES=libabbackends.la
libabbackends_la_SOURCES=dummy.c
libabbackends_la_LIBADD=@aqbanking_plugins_backends_libs@

sources:
	for d in $(DIST_SUBDIRS); do \
	  $(MAKE) -C $$d sources; \
	done

built_sources:
	for d in $(DIST_SUBDIRS); do \
	  $(MAKE) -C $$d built_sources; \
	done ;


cppcheck:
	for d in $(SUBDIRS); do \
	  $(MAKE) -C $$d cppcheck; \
	done

typefiles:
	for d in $(SUBDIRS); do \
	  $(MAKE) -C $$d typefiles; \
	done

typedefs:
	for d in $(SUBDIRS); do \
	  $(MAKE) -C $$d typedefs; \
	done

