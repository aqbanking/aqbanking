SUBDIRS=plugin

EXTRA_DIST=ChangeLog README aqgeldkarte.m4 version.h.in

bin_SCRIPTS=aqgeldkarte-config

iheaderdir=${includedir}/aqgeldkarte
iheader_HEADERS=\
  version.h
BUILT_SOURCES = version.h
CLEANFILES = $(BUILT_SOURCES) aqgeldkarte-config aqgeldkarte-config.in

aqgeldkarte-config: aqgeldkarte-config.in
	echo "#! /bin/sh" >aqgeldkarte-config && \
	echo 'dir="@prefix@"' >>aqgeldkarte-config && \
	cat aqgeldkarte-config.in >>aqgeldkarte-config && \
	chmod a+x aqgeldkarte-config

sources:
	for d in $(SUBDIRS); do \
	  make -C $$d sources; \
	done



