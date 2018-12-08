SUBDIRS = $(aqbanking_plugins_backends_dirs) aqnone
DIST_SUBDIRS = aqhbci aqofxconnect aqnone aqebics aqpaypal

noinst_LTLIBRARIES=libabbackends.la
libabbackends_la_SOURCES=dummy.c
libabbackends_la_LIBADD=@aqbanking_plugins_backends_libs@ aqnone/libaqnone.la

sources:
	for d in $(DIST_SUBDIRS); do \
	  $(MAKE) -C $$d sources; \
	done

built_sources:
	for d in $(DIST_SUBDIRS); do \
	  $(MAKE) -C $$d built_sources; \
	done ;
