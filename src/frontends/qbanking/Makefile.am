SUBDIRS=prg widgets dialogs views po

INCLUDES = \
  -I$(top_builddir)/headers \
  -I$(srcdir)/dialogs -I$(srcdir)/widgets -I$(srcdir)/prg -I$(srcdir)/views \
  -Idialogs -Iwidgets -Iprg -Iviews \
  $(all_includes)

DEFS += -DPKGDATADIR=\"$(pkgdatadir)\"

lib_LTLIBRARIES=libqbanking.la

bin_SCRIPTS=qbanking-config
M4dir = $(datadir)/aclocal
dist_M4_DATA=qbanking.m4

iheaderdir = $(includedir)/qbanking
iheader_HEADERS=\
 qbflagstaff.h \
 qbanking.h \
 qbwaitcallback.h \
 qbwcb_fast.h \
 qbwcb_simple.h \
 qbwcb_progress.h \
 banking.h

noinst_HEADERS = \
 qbanking_p.h \
 banking_p.h

libqbanking_la_SOURCES=\
 qbflagstaff.cpp \
 qbanking.cpp \
 qbwaitcallback.cpp \
 qbwcb_fast.cpp \
 qbwcb_simple.cpp \
 qbwcb_progress.cpp \
 banking.cpp

nodist_libqbanking_la_SOURCES=\
 qbflagstaff.moc.cpp

qbanking-config: qbanking-config.in
	echo "#! /bin/sh" >qbanking-config && \
	cat qbanking-config.in >>qbanking-config && \
	chmod a+x qbanking-config

BUILT_SOURCES = $(nodist_libqbanking_la_SOURCES) version.h
CLEANFILES = $(BUILT_SOURCES) i18nsources qbanking-config qbanking-config.in ressource.rc

libqbanking_la_LIBADD=\
 widgets/libwidgets.la \
 dialogs/libdialogs.la \
 views/libviews.la \
 $(aqbanking_internal_libs) $(gwenhywfar_libs) $(qt3_libs)

libqbanking_la_LDFLAGS=\
 -version-info @QBANKING_SO_CURRENT@:@QBANKING_SO_REVISION@:@QBANKING_SO_AGE@


sources:
	for f in $(libqbanking_la_SOURCES); do \
	  echo $(subdir)/$$f >>$(top_srcdir)/qtsources; \
	done
	for d in $(SUBDIRS); do \
	  make -C $$d sources; \
	done ;

%.moc.cpp: %.h
	@qt3_moc@ -o $@ $<


###------------------------------------------------------------------------
# DLL build rules for Windows/mingw32
if IS_WINDOWS

# You only need to setup these variables here, the rest is done
# automatically
LIBRARY_NAME = qbanking32_$(QBANKING_SO_EFFECTIVE)
DLLLDLIBS = \
  $(aqbanking_internal_libs) \
  -L/mingw/lib $(gwenhywfar_libs) \
  $(qt3_libs) \
  -lstdc++
DLLLDFLAGS = $(STRIPALL) $(LDFLAGS) -mwindows
DLLSRC_LIBLA = $(lib_LTLIBRARIES)

#--------------------------------------------------------------
# You should not have to make any modifications below this point

DLLNAME = $(LIBRARY_NAME).dll
DLLEXP_LIB = $(LIBRARY_NAME).lib
DLLEXP_DEF = $(LIBRARY_NAME).def
DLLSRC_LIB = $(DLLSRC_LIBLA:.la=.a)

DLLTOOL=dlltool
DLLWRAP=dllwrap
WINDRES=windres

# Variables so that the DLL and the LIB are installed correctly
dlldir = $(WIN_PATH_WINDOWS_MINGW)
dll_SCRIPTS = $(DLLNAME)
dlllibdir = $(libdir)
dlllib_SCRIPTS = $(DLLEXP_LIB)

ressource.o: ressource.rc
	$(WINDRES) -i $< -o $@

$(DLLNAME) $(DLLEXP_LIB): $(DLLSRC_LIBLA) ressource.o
	rm -rf dlldir
	mkdir -p dlldir && \
	( \
	  if test -r $(DLLSRC_LIB); then \
	    SRCLIB=$(DLLSRC_LIB); \
	  else \
	    SRCLIB=.libs/$(DLLSRC_LIB); \
	  fi; \
	  if ls .libs/*.o 2>/dev/null; then cp .libs/*.o dlldir; fi ; \
	  cd dlldir && ar x "../$${SRCLIB}" && \
	 $(DLLTOOL) -e ../exports.o *.o && \
	 $(DLLWRAP) \
	   --export-all \
	   --output-def ../$(DLLEXP_DEF) \
	   --implib ../$(DLLEXP_LIB) \
	   --driver-name $(CXX) \
	   -o ../$(DLLNAME) \
	     *.o ../exports.o \
	     ../ressource.o $(DLLLDFLAGS) $(DLLLDLIBS) \
	) \
	&& rm -rf dlldir

CLEANFILES += $(DLLNAME) $(DLLEXP_LIB) $(DLLEXP_DEF)

endif
# IS_WINDOWS
###------------------------------------------------------------------------


# This section is for a qt4 environment
qt4-port:
	for A in $(libqbanking_la_SOURCES) $(HEADERS); do \
	 $(YES) | $(QT3TO4) $$A; \
	done
	for d in $(SUBDIRS); do \
	  make -C $$d qt4-port; \
	done
# For qt3 compatibility of qt4
DEFS += -DQT3_SUPPORT

built_sources: $(BUILT_SOURCES)
	for d in $(SUBDIRS); do \
	  make -C $$d built_sources; \
	done ;

