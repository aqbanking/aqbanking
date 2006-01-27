INCLUDES=-I$(top_srcdir) -I$(top_srcdir)/headers @all_includes@

SUBDIRS=plugin ui

EXTRA_DIST=aqyellownet.m4

bin_SCRIPTS=aqyellownet-config
aclocaldir = $(datadir)/aclocal
dist_aclocal_DATA = aqyellownet.m4

aqyellownet-config: aqyellownet-config.in
	echo "#! /bin/sh" >aqyellownet-config && \
	echo 'dir="@prefix@"' >>aqyellownet-config && \
	cat aqyellownet-config.in >>aqyellownet-config && \
	chmod a+x aqyellownet-config


sources:
	for d in $(SUBDIRS); do \
	  make -C $$d sources; \
	done

built_sources: $(BUILT_SOURCES)
	for d in $(SUBDIRS); do \
	  make -C $$d built_sources; \
	done ;
