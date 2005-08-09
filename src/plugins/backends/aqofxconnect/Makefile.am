SUBDIRS=plugin ui

EXTRA_DIST=ChangeLog README aqofxconnect.m4 version.h.in

bin_SCRIPTS=aqofxconnect-config
M4dir=${prefix}/share/aclocal
dist_M4_DATA=aqofxconnect.m4

iheaderdir=${includedir}/aqofxconnect
iheader_HEADERS=\
  version.h
BUILT_SOURCES = version.h
CLEANFILES = $(BUILT_SOURCES) aqofxconnect-config aqofxconnect-config.in

aqofxconnect-config: aqofxconnect-config.in
	echo "#! /bin/sh" >aqofxconnect-config && \
	echo 'dir="@prefix@"' >>aqofxconnect-config && \
	cat aqofxconnect-config.in >>aqofxconnect-config && \
	chmod a+x aqofxconnect-config

sources:
	for d in $(SUBDIRS); do \
	  make -C $$d sources; \
	done



