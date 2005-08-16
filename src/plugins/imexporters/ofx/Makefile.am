SUBDIRS=profiles

# Include the source code files here as well. It doesnt hurt to
# include them twice in the dist target.
EXTRA_DIST=ofx.c ofx_p.h ofx.xml.in

#####
# The complete rest of the file is used only if HAVE_OFX is enabled.
#####
if WITH_OFX_PLUGIN

INCLUDES = -I$(top_builddir)/headers $(all_includes)
#EXTRA_DIST=README

imexporterplugindir = $(aqbanking_plugindir)/imexporters

imexporterplugin_LTLIBRARIES=ofx.la
imexporterplugin_DATA=ofx.xml

noinst_HEADERS=\
 ofx_p.h

ofx_la_SOURCES=\
 ofx.c

ofx_la_LIBADD=$(aqbanking_internal_libs) -lofx $(gwenhywfar_libs) 
ofx_la_LDFLAGS=@STRIPALL@ \
  -module -omit-version

sources:
	for f in $(ofx_la_SOURCES); do \
	  echo $(subdir)/$$f >>$(top_srcdir)/i18nsources; \
	done


# dll stuff
if IS_WINDOWS

# you only need to setup these variables here, the rest is done automatically
LIBRARY_NAME = ofx
DLLLDLIBS = $(aqbanking_internal_libs) -L/mingw/lib $(gwenhywfar_libs) 
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



endif
#####
# WITH_OFX_PLUGIN
#####
