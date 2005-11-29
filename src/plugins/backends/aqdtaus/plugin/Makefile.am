INCLUDES=-I$(top_builddir)/headers @all_includes@

localedir=$(datadir)/locale
DEFS += -DLOCALEDIR=\"$(localedir)\"

EXTRA_DIST=aqdtaus.xml.in

#CLEANFILES =

plugindir=@aqbanking_plugindir@/providers
plugin_LTLIBRARIES=aqdtaus.la
if IS_WINDOWS
plugin_DATA = aqdtaus.xml aqdtaus.dll
else
plugin_DATA=aqdtaus.xml
endif


lib_LTLIBRARIES=libaqdtaus.la 

noinst_HEADERS=\
 account_p.h \
 aqdtaus_l.h \
 job_p.h \
 provider_p.h


iheaderdir=${includedir}/aqdtaus
iheader_HEADERS=\
 account.h \
 aqdtaus.h \
 job.h \
 provider.h

libaqdtaus_la_SOURCES= \
 account.c \
 job.c \
 provider.c

aqdtaus_la_SOURCES= \
 aqdtaus-plugin.c

libaqdtaus_la_LIBADD= $(aqbanking_internal_libs) $(gwenhywfar_libs)
aqdtaus_la_LIBADD= \
  libaqdtaus.la \
  $(aqbanking_internal_libs) \
  $(gwenhywfar_libs)
libaqdtaus_la_LDFLAGS = -no-undefined -module \
  -version-info @AQDTAUS_SO_CURRENT@:@AQDTAUS_SO_REVISION@:@AQDTAUS_SO_AGE@
aqdtaus_la_LDFLAGS = -no-undefined -module \
  -version-info @AQDTAUS_SO_CURRENT@:@AQDTAUS_SO_REVISION@:@AQDTAUS_SO_AGE@


sources:
	for f in $(libaqdtaus_la_SOURCES); do \
	  echo $(subdir)/$$f >>$(top_srcdir)/i18nsources; \
	done
	for d in $(SUBDIRS); do \
	  make -C $$d sources; \
	done



# dll stuff
if IS_WINDOWS

# you only need to setup these variables here, the rest is done automatically
LIBRARY_NAME = aqdtaus32_$(AQDTAUS_SO_EFFECTIVE)
DLLLDLIBS = $(aqbanking_internal_libs) \
  -L/mingw/lib $(gwenhywfar_libs)
DLLLDFLAGS = $(STRIPALL) $(LDFLAGS)

PLUGIN_LIBRARY_NAME = aqdtaus

###------------------------------------------------------------------------
# You should not have to make any modifications below this point
#

DLLNAME = $(LIBRARY_NAME).dll
DLLEXP_LIB = $(LIBRARY_NAME).lib
DLLEXP_DEF = $(LIBRARY_NAME).def
DLLSRC_LIB = $(lib_LTLIBRARIES:.la=.a)
DLLSRC_LIBLA = $(lib_LTLIBRARIES)

PLUGIN_DLLNAME = $(PLUGIN_LIBRARY_NAME).dll
PLUGIN_DLLEXP_LIB = $(PLUGIN_LIBRARY_NAME).lib
PLUGIN_DLLEXP_DEF = $(PLUGIN_LIBRARY_NAME).def
PLUGIN_DLLSRC_LIB = $(plugin_LTLIBRARIES:.la=.a)
PLUGIN_DLLSRC_LIBLA = $(plugin_LTLIBRARIES)

DLLTOOL=dlltool
DLLWRAP=dllwrap
WINDRES=windres

# Variables so that the DLL and the LIB are installed correctly
dlldir = $(WIN_PATH_WINDOWS_MINGW)
dll_SCRIPTS = $(DLLNAME)
dlllibdir = $(libdir)
dlllib_SCRIPTS = $(DLLEXP_LIB) $(PLUGIN_DLLEXP_LIB)

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

$(PLUGIN_DLLNAME) $(PLUGIN_DLLEXP_LIB): $(PLUGIN_DLLSRC_LIBLA) ressource.o $(DLLNAME)
	mkdir -p dlldir && \
	( \
	  if test -r $(PLUGIN_DLLSRC_LIB); then \
	    SRCLIB=$(PLUGIN_DLLSRC_LIB); \
	  else \
	    SRCLIB=.libs/$(PLUGIN_DLLSRC_LIB); \
	  fi; \
	  cd dlldir && ar x "../$${SRCLIB}" && \
	 ( ls ../.libs/*.o && cp ../.libs/*.o .) && \
	 $(DLLTOOL) -e ../exports.o *.o && \
	 $(DLLWRAP) \
	   --export-all \
	   --output-def ../$(PLUGIN_DLLEXP_DEF) \
	   --implib ../$(PLUGIN_DLLEXP_LIB) \
	   --driver-name $(CC) \
	   -o ../$(PLUGIN_DLLNAME) \
	     *.o ../exports.o \
	     ../ressource.o $(DLLLDFLAGS) -L.. $(aqdtaus_internal_libs) $(DLLLDLIBS) \
	) \
	&& rm -rf dlldir

CLEANFILES = $(DLLNAME) $(DLLEXP_LIB) $(DLLEXP_DEF)

endif
# IS_WINDOWS


built_sources:

