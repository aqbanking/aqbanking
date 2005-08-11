SUBDIRS=profiles

INCLUDES = -I$(top_builddir)/headers $(all_includes)
#EXTRA_DIST=README

imexporterplugindir = $(aqbanking_plugindir)/imexporters

imexporterplugin_LTLIBRARIES=eri2.la
if IS_WINDOWS
imexporterplugin_DATA=eri2.xml eri2.dll
else
imexporterplugin_DATA=eri2.xml
endif

imexporterdatadir=$(aqbanking_pkgdatadir)/imexporters/eri2
dist_imexporterdata_DATA=eriformat.xml

noinst_HEADERS=\
 eri2_p.h

eri2_la_SOURCES=\
 eri2.c

eri2_la_LIBADD=-L../../../libs/aqbanking -laqbanking @gwenhywfar_libs@ 
eri2_la_LDFLAGS=@STRIPALL@ -module -omit-version

sources:
	for f in $(eri2_la_SOURCES); do \
	  echo $(subdir)/$$f >>$(top_srcdir)/i18nsources; \
	done


# dll stuff
if IS_WINDOWS

# you only need to setup these variables here, the rest is done automatically
LIBRARY_NAME = eri2
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


