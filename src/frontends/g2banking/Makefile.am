## Process this file with automake to produce Makefile.in

SUBDIRS=widgets dialogs views

LIBDATADIR=$(g2banking_data)
g2_gladedir=$(g2banking_data)/glade2
dist_g2_glade_DATA=g2banking.glade

AM_CFLAGS=@visibility_cflags@

DEFS += -DBUILDING_GBANKING \
  -DPACKAGE_DATA_DIR=\""$(LIBDATADIR)\"" \
  -DPACKAGE_LOCALE_DIR=\""$(datadir)/locale\"" \
  -DPACKAGE_GLADE_DIR=\""$(g2_gladedir)\""

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

BUILT_SOURCES=version.h

lib_LTLIBRARIES=libg2banking.la

noinst_HEADERS=gbanking_p.h gbanking_l.h

iheaderdir=${includedir}/g2banking
iheader_HEADERS=\
 gbanking.h

libg2banking_la_SOURCES = \
 gbanking.c

libg2banking_la_LIBADD = \
 views/libviews.la \
 widgets/libwidgets.la \
 dialogs/libdialogs.la \
 $(aqbanking_internal_libs) \
 $(GTK2_LIBS) \
 $(LIBGLADE2_LIBS) \
 $(gwenhywfar_libs) \
 $(all_libraries)

libg2banking_la_LDFLAGS = -no-undefined \
  -version-info @G2BANKING_SO_CURRENT@:@G2BANKING_SO_REVISION@:@G2BANKING_SO_AGE@


CLEANFILES=g2banking-config g2banking-config.in ressource.rc $(BUILT_SOURCES)

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
TESTS = testlib

CLEANFILES += $(BUILT_SOURCES)

built_sources: $(BUILT_SOURCES)

