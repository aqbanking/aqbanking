SUBDIRS=plugin tools media ui

CLEANFILES = $(BUILT_SOURCES) aqhbci-config aqhbci-config.in

bin_SCRIPTS=aqhbci-config
M4dir=${prefix}/share/aclocal
dist_M4_DATA=aqhbci.m4

aqhbci-config: aqhbci-config.in
	echo "#! /bin/sh" >aqhbci-config && \
	cat aqhbci-config.in >>aqhbci-config && \
	chmod a+x aqhbci-config

sources:
	for d in $(SUBDIRS); do \
	  $(MAKE) -C $$d sources; \
	done

built_sources:
	for d in $(SUBDIRS); do \
	  $(MAKE) -C $$d built_sources; \
	done ;
