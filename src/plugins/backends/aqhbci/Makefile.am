SUBDIRS=plugin tools media ui

CLEANFILES = $(BUILT_SOURCES) aqhbci-config aqhbci-config.in

bin_SCRIPTS=aqhbci-config

aqhbci-config: aqhbci-config.in
	echo "#! /bin/sh" >aqhbci-config && \
	echo 'dir="@prefix@"' >>aqhbci-config && \
	cat aqhbci-config.in >>aqhbci-config && \
	chmod a+x aqhbci-config

sources:
	for d in $(SUBDIRS); do \
	  make -C $$d sources; \
	done
