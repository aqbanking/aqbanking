
INCLUDES = -I$(top_srcdir)/src/libs/aqbanking \
  -I$(top_builddir)/headers $(all_includes)
#EXTRA_DIST=README

bin_SCRIPTS=cbanking-config
M4dir = $(datadir)/aclocal
dist_M4_DATA=cbanking.m4

cbanking-config: cbanking-config.in
	echo "#! /bin/sh" >cbanking-config && \
	cat cbanking-config.in >>cbanking-config && \
	chmod a+x cbanking-config

CLEANFILES=cbanking-config cbanking-config.in

lib_LTLIBRARIES=libcbanking.la

iheaderdir=${includedir}/cbanking
iheader_HEADERS=cbanking.h

noinst_HEADERS=\
 cbanking_l.h \
 cbanking_p.h \
 progress_l.h \
 progress_p.h

libcbanking_la_SOURCES=\
 cbanking.c \
 progress.c

libcbanking_la_LIBADD = \
  $(aqbanking_internal_libs) $(gwenhywfar_libs)
libcbanking_la_LDFLAGS = $(STRIPALL) \
  -version-info @AQBANKING_SO_CURRENT@:@AQBANKING_SO_REVISION@:@AQBANKING_SO_AGE@


sources:
	for f in $(libcbanking_la_SOURCES); do \
	  echo $(subdir)/$$f >>$(top_srcdir)/i18nsources; \
	done


# Build and link a test program to verify the linker flags
noinst_PROGRAMS = testlib
testlib_SOURCES = testlib.c
testlib_LDADD = libcbanking.la


# dll stuff
if IS_WINDOWS

# you only need to setup these variables here, the rest is done automatically
LIBRARY_NAME = cbanking32_$(CBANKING_SO_EFFECTIVE)
DLLLDLIBS = $(aqbanking_internal_libs) -L/mingw/lib $(gwenhywfar_libs)
DLLLDFLAGS = $(STRIPALL) $(LDFLAGS)
DLLSRC_LIBLA = $(lib_LTLIBRARIES)

###------------------------------------------------------------------------
# You should not have to make any modifications below this point
#

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
	   --driver-name $(CC) \
	   -o ../$(DLLNAME) \
	     *.o ../exports.o \
	     ../ressource.o $(DLLLDFLAGS) $(DLLLDLIBS) \
	) \
	&& rm -rf dlldir

CLEANFILES += $(DLLNAME) $(DLLEXP_LIB) $(DLLEXP_DEF)

endif
# IS_WINDOWS

built_sources:

