SUBDIRS=plugin ui

EXTRA_DIST=ChangeLog README aqgeldkarte.m4 version.h.in

bin_SCRIPTS=aqgeldkarte-config
M4dir=${prefix}/share/aclocal
dist_M4_DATA=aqgeldkarte.m4

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

built_sources: $(BUILT_SOURCES)
	for d in $(SUBDIRS); do \
	  make -C $$d built_sources; \
	done ;


