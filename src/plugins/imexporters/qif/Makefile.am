SUBDIRS=profiles

INCLUDES = -I$(top_srcdir)/src/libs/aqbanking $(all_includes)
#EXTRA_DIST=README

imexporterplugindir = $(aqbanking_plugindir)/imexporters

imexporterplugin_LTLIBRARIES=qif.la
if IS_WINDOWS
imexporterplugin_DATA=qif.xml qif.dll
else
imexporterplugin_DATA=qif.xml
endif

noinst_HEADERS=\
 qif_p.h

qif_la_SOURCES=\
 qif.c

qif_la_LIBADD=-L$(top_builddir)/src/libs/aqbanking -laqbanking \
  $(gwenhywfar_libs)
qif_la_LDFLAGS=@STRIPALL@ -module -omit-version

sources:
	for f in $(qif_la_SOURCES); do \
	  echo $(subdir)/$$f >>$(top_srcdir)/i18nsources; \
	done


# dll stuff
if IS_WINDOWS

# you only need to setup these variables here, the rest is done automatically
LIBRARY_NAME = qif
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


