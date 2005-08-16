SUBDIRS=msglayer joblayer accountjobs applayer banking

EXTRA_DIST=aqhbci.xml.in header.xml.in

INCLUDES = -I$(top_builddir)/headers $(all_includes) \
 -I$(srcdir)/joblayer

BUILT_SOURCES = version.h

CLEANFILES = $(BUILT_SOURCES) hbci.xml


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

xmldatadir = $(aqhbci_data)/xml
xmldata_DATA=hbci.xml

hbci.xml: header.xml applayer/xml/base.xml accountjobs/accountjobs.xml
	$(XMLMERGE) --compact --header -v header.xml applayer/xml/base.xml accountjobs/accountjobs.xml -o $@


sources:
	for f in $(libaqhbci_la_SOURCES); do \
	  echo $(subdir)/$$f >>$(top_srcdir)/i18nsources; \
	done
	for d in $(SUBDIRS); do \
	  make -C $$d sources; \
	done

libaqhbci_la_LIBADD = \
  msglayer/libhbcimsg.la \
  joblayer/libhbcijob.la \
  accountjobs/libhbciaccjobs.la \
  applayer/libhbciapp.la \
  banking/libhbcibanking.la \
  -L$(top_builddir)/src/libs/aqbanking $(aqbanking_libs) \
  $(gwenhywfar_libs)


libaqhbci_la_LDFLAGS=\
  -version-info @AQHBCI_SO_CURRENT@:@AQHBCI_SO_REVISION@:@AQHBCI_SO_AGE@


aqhbci_la_SOURCES=aqhbci-plugin.c
#aqhbci_la_SOURCES=dummy.c
aqhbci_la_LIBADD = \
  $(aqhbci_internal_libs) \
  $(aqbanking_internal_libs) \
  $(gwenhywfar_libs)
aqhbci_la_LDFLAGS=-module \
  -version-info @AQHBCI_SO_CURRENT@:@AQHBCI_SO_REVISION@:@AQHBCI_SO_AGE@


# dll stuff
if IS_WINDOWS

# you only need to setup these variables here, the rest is done automatically
LIBRARY_NAME = aqhbci32_$(AQHBCI_SO_EFFECTIVE)
DLLLDLIBS = \
  $(aqbanking_internal_libs) \
  -L/mingw/lib $(gwenhywfar_libs)
DLLLDFLAGS = $(STRIPALL) $(LDFLAGS)

PLUGIN_LIBRARY_NAME = aqhbci

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


# This target contains an ugly hack: Unfortunately the *.a files don't 
# include object files from the current level. so we have to copy them
# manually. However, the object file aqhbci-plugin.o must not be copied
# for the library dll but only for the plugin dll. So for now we just
# delete the file aqhbci-plugin.o from dlldir...
$(DLLNAME): $(DLLSRC_LIBLA) ressource.o
	test -d dlldir && rm -rf dlldir
	mkdir -p dlldir && \
	( \
	  if test -r $(DLLSRC_LIB); then \
	    SRCLIB=$(DLLSRC_LIB); \
	  else \
	    SRCLIB=.libs/$(DLLSRC_LIB); \
	  fi; \
	  if ls .libs/*.o 2>/dev/null; then cp .libs/*.o dlldir; fi ; \
	  rm -f dlldir/aqhbci-plugin.o 2>/dev/null; \
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

$(PLUGIN_DLLNAME): $(PLUGIN_DLLSRC_LIBLA) ressource.o $(DLLNAME)
	test -d dlldir && rm -rf dlldir
	mkdir -p dlldir && \
	( \
	  if test -r $(PLUGIN_DLLSRC_LIB); then \
	    SRCLIB=$(PLUGIN_DLLSRC_LIB); \
	  else \
	    SRCLIB=.libs/$(PLUGIN_DLLSRC_LIB); \
	  fi; \
	  if ls .libs/*.o 2>/dev/null; then cp .libs/*.o dlldir; fi ; \
	  cd dlldir && ar x "../$${SRCLIB}" && \
	 $(DLLTOOL) -e ../exports.o *.o && \
	 $(DLLWRAP) \
	   --export-all \
	   --output-def ../$(PLUGIN_DLLEXP_DEF) \
	   --implib ../$(PLUGIN_DLLEXP_LIB) \
	   --driver-name $(CC) \
	   -o ../$(PLUGIN_DLLNAME) \
	     *.o ../exports.o \
	     ../ressource.o $(DLLLDFLAGS) $(aqhbci_internal_libs) $(DLLLDLIBS) \
	) \
	&& rm -rf dlldir

CLEANFILES += $(DLLNAME) $(DLLEXP_LIB) $(DLLEXP_DEF)

endif
# IS_WINDOWS

built_sources: $(BUILT_SOURCES)

