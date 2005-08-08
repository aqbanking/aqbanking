SUBDIRS=plugin

EXTRA_DIST=ChangeLog README aqdtaus.m4 version.h.in

bin_SCRIPTS=aqdtaus-config

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



