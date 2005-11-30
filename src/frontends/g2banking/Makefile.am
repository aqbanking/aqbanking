## Process this file with automake to produce Makefile.in

SUBDIRS=widgets dialogs views

LIBDATADIR="$(datadir)/g2banking"
DEFS += \
  -DPACKAGE_DATA_DIR=\""$(LIBDATADIR)\"" \
  -DPACKAGE_LOCALE_DIR=\""$(datadir)/locale\""

INCLUDES = -I$(srcdir)/widgets -I$(srcdir)/dialogs -I$(srcdir)/views \
  -I$(top_builddir)/headers $(all_includes)

bin_SCRIPTS=g2banking-config
M4dir=$(datadir)/aclocal
dist_M4_DATA=g2banking.m4

g2banking-config: g2banking-config.in
	echo "#! /bin/sh" >g2banking-config && \
	echo 'dir="@prefix@"' >>g2banking-config && \
	cat $<  >> g2banking-config && \
	chmod a+x g2banking-config

EXTRA_DIST = g2banking.glade
GLADE_SOURCES=support.c interface.c callbacks.c
GLADE_HDRS=support.h interface.h callbacks.h

BUILT_SOURCES=$(GLADE_SOURCES) $(GLADE_HDRS) version.h

lib_LTLIBRARIES=libg2banking.la

noinst_HEADERS=gbanking_p.h
nodist_noinst_HEADERS=$(GLADE_HDRS)

iheaderdir=${includedir}/g2banking
iheader_HEADERS=\
 gbanking.h

libg2banking_la_SOURCES = \
 gbanking.c

nodist_libg2banking_la_SOURCES=$(GLADE_SOURCES)

libg2banking_la_LIBADD = \
 views/libviews.la \
 widgets/libwidgets.la \
 dialogs/libdialogs.la \
 $(aqbanking_internal_libs) \
 $(GTK2_LIBS) \
 $(gwenhywfar_libs) \
 $(all_libraries)

libg2banking_la_LDFLAGS = -no-undefined \
  -version-info @G2BANKING_SO_CURRENT@:@G2BANKING_SO_REVISION@:@G2BANKING_SO_AGE@


CLEANFILES=g2banking-config g2banking-config.in ressource.rc $(BUILT_SOURCES)

$(GLADE_SOURCES) $(GLADE_HDRS): g2banking.glade
	rm -Rf tmp.glade >/dev/null
	mkdir tmp.glade
	cp $< tmp.glade
	( cd tmp.glade && $(GLADE2) -w g2banking.glade ) 
	for f in $(GLADE_SOURCES) $(GLADE_HDRS); do \
	      cp tmp.glade/src/$$f . ; \
	done
	rm -Rf tmp.glade >/dev/null


install-data-local:
	@$(NORMAL_INSTALL)
	if test -d $(srcdir)/pixmaps; then \
	  $(mkinstalldirs) $(DESTDIR)$(LIBDATADIR)/pixmaps; \
	  for pixmap in $(srcdir)/pixmaps/*; do \
	    if test -f $$pixmap; then \
	      $(INSTALL_DATA) $$pixmap $(DESTDIR)$(LIBDATADIR)/pixmaps; \
	    fi \
	  done \
	fi

dist-hook:
	if test -d pixmaps; then \
	  mkdir $(distdir)/pixmaps; \
	  for pixmap in pixmaps/*; do \
	    if test -f $$pixmap; then \
	      cp -p $$pixmap $(distdir)/pixmaps; \
	    fi \
	  done \
	fi

# Add the linker flags for windows resource file
libg2banking_la_DEPENDENCIES = \
 views/libviews.la \
 widgets/libwidgets.la \
 dialogs/libdialogs.la \
 $(aqbanking_internal_libs)
# if IS_WINDOWS
if IS_WINDOWS
libg2banking_la_DEPENDENCIES += ressource.o
libg2banking_la_LDFLAGS += -Wl,ressource.o
ressource.o: ressource.rc
	$(RC) -o $@ $<
endif
# IS_WINDOWS
# End of linker flags for windows resource file

sources:
	for f in $(libg2banking_la_SOURCES); do \
	  echo $(subdir)/$$f >>$(top_srcdir)/i18nsources; \
	done
	for d in $(SUBDIRS); do \
	  make -C $$d sources; \
	done


# Build and link a test program to verify the linker flags
noinst_PROGRAMS = testlib
testlib_SOURCES = testlib.c
testlib_LDADD = libg2banking.la

CLEANFILES += $(BUILT_SOURCES)

built_sources: $(BUILT_SOURCES)

