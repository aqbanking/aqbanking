## Process this file with automake to produce Makefile.in

SUBDIRS=widgets dialogs views

LIBDATADIR="$(datadir)/g2banking"
DEFS += \
  -DPACKAGE_DATA_DIR=\""$(LIBDATADIR)\"" \
  -DPACKAGE_LOCALE_DIR=\""$(datadir)/locale\""

INCLUDES = -Iwidgets -Idialogs -Iviews -I$(top_builddir)/headers $(all_includes)

bin_SCRIPTS=g2banking-config
M4dir=${prefix}/share/aclocal
M4_DATA=g2banking.m4

g2banking-config: g2banking-config.in
	echo "#! /bin/sh" >g2banking-config && \
	echo 'dir="@prefix@"' >>g2banking-config && \
	cat $< >> g2banking-config && \
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

libg2banking_la_LIBADD =\
 -L$(top_builddir)/src/libs/aqbanking $(aqbanking_internal_libs) \
 $(gwenhywfar_libs) \
 views/libviews.la \
 widgets/libwidgets.la \
 dialogs/libdialogs.la \
 $(all_libraries)

libg2banking_la_LDFLAGS =\
  -version-info @G2BANKING_SO_CURRENT@:@G2BANKING_SO_REVISION@:@G2BANKING_SO_AGE@


CLEANFILES=g2banking-config g2banking-config.in ressource.rc $(BUILT_SOURCES)

$(GLADE_SOURCES) $(GLADE_HDRS): g2banking.glade
	rm -Rf tmp.glade >/dev/null
	mkdir tmp.glade
	cp $< tmp.glade
	( cd tmp.glade && glade-2 -w g2banking.glade ) 
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


sources:
	for f in $(libg2banking_la_SOURCES); do \
	  echo $(subdir)/$$f >>$(top_srcdir)/i18nsources; \
	done
	for d in $(SUBDIRS); do \
	  make -C $$d sources; \
	done


# dll stuff
if IS_WINDOWS

# you only need to setup these variables here, the rest is done automatically
LIBRARY_NAME = g2banking32_$(G2BANKING_SO_EFFECTIVE)
DLLLDLIBS = \
  -L/mingw/lib @gwenhywfar_libs@ -L$(top_builddir)/src/libs/aqbanking -laqbanking

DLLLDFLAGS = $(STRIPALL) $(LDFLAGS)


###------------------------------------------------------------------------
# You should not have to make any modifications below this point
#

DLLNAME = $(LIBRARY_NAME).dll
DLLEXP_LIB = $(LIBRARY_NAME).lib
DLLEXP_DEF = $(LIBRARY_NAME).def
DLLSRC_LIB = $(lib_LTLIBRARIES:.la=.a)
DLLSRC_LIBLA = $(lib_LTLIBRARIES)

all-local: dll

install-exec-local: dll-install

DLLTOOL=dlltool
DLLWRAP=dllwrap
WINDRES=windres


dll: $(DLLNAME)

dll-install: dll
	$(INSTALL) -D $(DLLNAME) $(DESTDIR)$(WIN_PATH_WINDOWS_MINGW)/$(DLLNAME)

ressource.o: ressource.rc
	$(WINDRES) -i $< -o $@


$(DLLNAME): $(DLLSRC_LIBLA) ressource.o
	mkdir -p dlldir && \
	( \
	  if test -r $(DLLSRC_LIB); then \
	    SRCLIB=$(DLLSRC_LIB); \
	  else \
	    SRCLIB=.libs/$(DLLSRC_LIB); \
	  fi; \
	  cd dlldir && ar x "../$${SRCLIB}" && \
	 ( ls ../.libs/*.o && cp ../.libs/*.o .) && \
	 $(DLLTOOL) -e ../exports.o *.o && \
	 $(DLLWRAP) \
	   --export-all \
	   --output-def ../$(DLLEXP_DEF) \
	   --implib ../$(DLLEXP_LIB) \
	   --driver-name $(CC) \
	   -o ../$(DLLNAME) \
	     *.o ../exports.o \
	     ../ressource.o $(DLLLDFLAGS) $(DLLLDLIBS) \
	)

endif
# IS_WINDOWS


CLEANFILES += $(DLLNAME) $(DLLEXP_LIB) $(DLLEXP_DEF) $(BUILT_SOURCES)

