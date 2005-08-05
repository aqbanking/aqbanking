INCLUDES=-I$(top_builddir)/headers $(all_includes)

lib_LTLIBRARIES=libaqbankingpp.la

#DEFS += -DAQBANKING_PLUGINS=\"$(aqbanking_plugindir)\"

noinst_HEADERS=\
 banking_p.h

iheaderdir=$(includedir)/aqbanking++
iheader_HEADERS=\
 banking.h

libaqbankingpp_la_SOURCES=\
 banking.cpp

libaqbankingpp_la_LIBADD = \
 -L$(top_builddir)/src/libs/aqbanking $(aqbanking_internal_libs) \
 $(gwenhywfar_libs)

sources:
	for f in $(libaqbankingpp_la_SOURCES); do \
	  echo $(subdir)/$$f >>$(top_srcdir)/i18nsources; \
	done

# dll stuff
if IS_WINDOWS

# you only need to setup these variables here, the rest is done automatically
LIBRARY_NAME = aqbankingpp32
DLLLDLIBS = \
  -L$(top_builddir)/src/libs/aqbanking $(aqbanking_internal_libs) \
  -L/mingw/lib $(gwenhywfar_libs) \
  -L/mingw/lib -lstdc++
DLLLDFLAGS = $(STRIPALL) $(LDFLAGS)

###------------------------------------------------------------------------
# You should not have to make any modifications below this point
#

DLLNAME = $(LIBRARY_NAME).dll
DLLEXP_LIB = $(LIBRARY_NAME).lib
DLLEXP_DEF = $(LIBRARY_NAME).def
DLLSRC_LIB = $(lib_LTLIBRARIES:.la=.a)
DLLSRC_LIBLA = $(lib_LTLIBRARIES)

all-local: dll

install-exec-local: dll-install

DLLTOOL=dlltool
DLLWRAP=dllwrap
WINDRES=windres

dll: $(DLLNAME)

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
	)

CLEANFILES = $(DLLNAME) $(DLLEXP_LIB) $(DLLEXP_DEF) 
clean-local:
	-rm -rf dlldir

endif
# IS_WINDOWS
