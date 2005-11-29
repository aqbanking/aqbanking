INCLUDES = -I$(top_builddir)/headers $(gwenhywfar_includes) $(all_includes)
EXTRA_DIST=README

dbioplugindir = $(gwenhywfar_plugins)/dbio
if IS_WINDOWS
dbioplugin_DATA=swift.xml swift.dll
else
dbioplugin_DATA=swift.xml
endif

dbioplugin_LTLIBRARIES=swift.la
noinst_HEADERS=\
  swift.h \
  swift_l.h \
  swift_p.h \
  swift940_l.h \
  swift940_p.h

swift_la_SOURCES=\
 swift.c \
 swift940.c

swift_la_LIBADD = \
  $(aqbanking_internal_libs) \
  $(gwenhywfar_libs)
  
swift_la_LDFLAGS = -no-undefined $(STRIPALL) -module -omit-version


sources:
	for f in $(swift_la_SOURCES); do \
	  echo $(subdir)/$$f >>$(top_srcdir)/i18nsources; \
	done


# dll stuff
if IS_WINDOWS

# you only need to setup these variables here, the rest is done automatically
LIBRARY_NAME = swift
DLLLDLIBS = $(aqbanking_internal_libs) -L/mingw/lib $(gwenhywfar_libs)
DLLLDFLAGS = $(STRIPALL) $(LDFLAGS)
DLLSRC_LIBLA = $(dbioplugin_LTLIBRARIES)

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

CLEANFILES = $(DLLNAME) $(DLLEXP_LIB) $(DLLEXP_DEF)

endif
# IS_WINDOWS


