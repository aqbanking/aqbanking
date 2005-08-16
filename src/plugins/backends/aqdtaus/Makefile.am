SUBDIRS=plugin ui

EXTRA_DIST=ChangeLog README aqdtaus.m4 version.h.in

bin_SCRIPTS=aqdtaus-config
M4dir=${prefix}/share/aclocal
dist_M4_DATA=aqdtaus.m4

iheaderdir=${includedir}/aqdtaus
iheader_HEADERS=\
  version.h
BUILT_SOURCES = version.h
CLEANFILES = $(BUILT_SOURCES) aqdtaus-config aqdtaus-config.in

aqdtaus-config: aqdtaus-config.in
	echo "#! /bin/sh" >aqdtaus-config && \
	echo 'dir="@prefix@"' >>aqdtaus-config && \
	cat aqdtaus-config.in >>aqdtaus-config && \
	chmod a+x aqdtaus-config

sources:
	for d in $(SUBDIRS); do \
	  make -C $$d sources; \
	done

built_sources: $(BUILT_SOURCES)
	for d in $(SUBDIRS); do \
	  make -C $$d built_sources; \
	done ;


