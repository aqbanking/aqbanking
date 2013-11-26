SUBDIRS = $(aqbanking_plugins_backends_dirs)
DIST_SUBDIRS = aqhbci aqofxconnect aqnone aqebics

noinst_LTLIBRARIES=libbackends.la
libbackends_la_SOURCES=dummy.c
libbackends_la_LIBADD=$(aqbanking_plugins_backends_libs)

sources:
	for d in $(DIST_SUBDIRS); do \
	  $(MAKE) -C $$d sources; \
	done

built_sources:
	for d in $(DIST_SUBDIRS); do \
	  $(MAKE) -C $$d built_sources; \
	done ;
