SUBDIRS=msglayer joblayer applayer banking

EXTRA_DIST=aqhbci.xml.in

INCLUDES = -I$(top_builddir)/headers $(all_includes) \
 -I$(srcdir)/joblayer

BUILT_SOURCES = version.h

CLEANFILES = $(BUILT_SOURCES)


plugindir= $(aqbanking_plugindir)/providers
plugin_LTLIBRARIES=aqhbci.la
if IS_WINDOWS
plugin_DATA = aqhbci.xml aqhbci.dll
else
plugin_DATA=aqhbci.xml
endif

iheaderdir=${includedir}/aqhbci
iheader_HEADERS=aqhbci.h

noinst_HEADERS = aqhbci_l.h

lib_LTLIBRARIES=libaqhbci.la
libaqhbci_la_SOURCES=dummy.c

sources:
	for f in $(libaqhbci_la_SOURCES); do \
	  echo $(subdir)/$$f >>$(top_srcdir)/i18nsources; \
	done
	for d in $(SUBDIRS); do \
	  make -C $$d sources; \
	done

libaqhbci_la_LIBADD=@gwenhywfar_libs@ -L$(top_builddir)/src/libs/aqbanking -laqbanking \
  msglayer/libhbcimsg.la \
  joblayer/libhbcijob.la \
  applayer/libhbciapp.la \
  banking/libhbcibanking.la

libaqhbci_la_LDFLAGS=\
  -version-info @AQHBCI_SO_CURRENT@:@AQHBCI_SO_REVISION@:@AQHBCI_SO_AGE@


aqhbci_la_SOURCES=aqhbci-plugin.c
#aqhbci_la_SOURCES=dummy.c
aqhbci_la_LIBADD = -L$(top_builddir)/src/libs $(aqhbci_internal_libs) $(gwenhywfar_libs) $(aqbanking_libs)
aqhbci_la_LDFLAGS=-module \
  -version-info @AQHBCI_SO_CURRENT@:@AQHBCI_SO_REVISION@:@AQHBCI_SO_AGE@


# dll stuff
if IS_WINDOWS

# you only need to setup these variables here, the rest is done automatically
LIBRARY_NAME = $(PACKAGE)32_$(AQHBCI_SO_EFFECTIVE)
DLLLDLIBS = -L/mingw/lib $(gwenhywfar_libs) $(aqbanking_libs)
DLLLDFLAGS = $(STRIPALL) $(LDFLAGS)

PLUGIN_LIBRARY_NAME = $(PACKAGE)

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

all-local: dll

install-exec-local: dll-install

DLLTOOL=dlltool
DLLWRAP=dllwrap
WINDRES=windres


dll: $(DLLNAME) $(PLUGIN_DLLNAME)

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
	 ( ls ../.libs/dummy.o && cp ../.libs/dummy.o .) && \
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

$(PLUGIN_DLLNAME): $(PLUGIN_DLLSRC_LIBLA) ressource.o $(DLLNAME)
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
	     ../ressource.o $(DLLLDFLAGS) -L../$(top_builddir)/src/libs $(aqhbci_internal_libs) $(DLLLDLIBS) \
	) \
	&& rm -rf dlldir

CLEANFILES += $(DLLNAME) $(DLLEXP_LIB) $(DLLEXP_DEF)

endif
# IS_WINDOWS


