
INCLUDES = -I$(top_srcdir)/src/libs/aqbanking \
  -I$(top_builddir)/headers $(all_includes)
#EXTRA_DIST=README

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
  -L$(top_builddir)/src/libs/aqbanking -laqbanking \
  $(gwenhywfar_libs)
libcbanking_la_LDFLAGS = $(STRIPALL) \
  -version-info @AQBANKING_SO_CURRENT@:@AQBANKING_SO_REVISION@:@AQBANKING_SO_AGE@


sources:
	for f in $(libcbanking_la_SOURCES); do \
	  echo $(subdir)/$$f >>$(top_srcdir)/i18nsources; \
	done


# dll stuff
if IS_WINDOWS

# you only need to setup these variables here, the rest is done automatically
LIBRARY_NAME = cbanking
DLLLDLIBS = -L/mingw/lib $(gwenhywfar_libs) -L../$(top_builddir)/src/libs/aqbanking $(aqbanking_internal_libs)
DLLLDFLAGS = $(STRIPALL) $(LDFLAGS)
DLLSRC_LIBLA = $(lib_LTLIBRARIES)

###------------------------------------------------------------------------
# You should not have to make any modifications below this point
#

DLLNAME = $(LIBRARY_NAME).dll
DLLEXP_LIB = $(LIBRARY_NAME).lib
DLLEXP_DEF = $(LIBRARY_NAME).def
DLLSRC_LIB = $(DLLSRC_LIBLA:.la=.a)

all-local: dll

DLLTOOL=dlltool
DLLWRAP=dllwrap
WINDRES=windres

dll: $(DLLNAME)

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
	) \
	&& rm -rf dlldir

CLEANFILES = $(DLLNAME) $(DLLEXP_LIB) $(DLLEXP_DEF)

endif
# IS_WINDOWS


