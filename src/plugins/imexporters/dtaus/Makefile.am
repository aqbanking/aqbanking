SUBDIRS=profiles

INCLUDES = $(all_includes)
EXTRA_DIST=README

imexporterplugindir=@aqbanking_plugindir@/imexporters

imexporterplugin_LTLIBRARIES=dtaus.la
if IS_WINDOWS
imexporterplugin_DATA=dtaus.xml dtaus.dll
else
imexporterplugin_DATA=dtaus.xml
endif

noinst_HEADERS=\
 dtaus_p.h

dtaus_la_SOURCES=\
 dtaus.c

dtaus_la_LIBADD=-L../../../libs/aqbanking -laqbanking @gwenhywfar_libs@ 
dtaus_la_LDFLAGS=@STRIPALL@ -module -omit-version


sources:
	for f in $(dtaus_la_SOURCES); do \
	  echo $(subdir)/$$f >>$(top_srcdir)/i18nsources; \
	done


# dll stuff
if IS_WINDOWS

# you only need to setup these variables here, the rest is done automatically
LIBRARY_NAME = dtaus
DLLLDLIBS = -L/mingw/lib $(gwenhywfar_libs) -L../$(top_builddir)/src/libs/aqbanking $(aqbanking_internal_libs)
DLLLDFLAGS = $(STRIPALL) $(LDFLAGS)
DLLSRC_LIBLA = $(imexporterplugin_LTLIBRARIES)

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


