SUBDIRS=jobs types

INCLUDES=-I${top_srcdir} -Itypes -Ijobs @all_includes@

lib_LTLIBRARIES=libaqbanking.la

# FIXME: Why don't all these go into config.h? It is necessary to
# include @DEFS@ here because -DHAVE_CONFIG_H is missing otherwise.
DEFS= @DEFS@ \
 -DAQBANKING_PLUGINS=\"@aqbanking_plugins@\" \
 -DDATADIR=\"$(aqbanking_data)\"

noinst_HEADERS=\
 account_l.h \
 account_p.h \
 banking_l.h \
 banking_p.h \
 imexporter_l.h \
 imexporter_p.h \
 job_l.h \
 job_p.h \
 provider_l.h \
 provider_p.h


iheaderdir=${includedir}/aqbanking
iheader_HEADERS=\
 account.h \
 banking.h \
 banking_be.h \
 error.h \
 imexporter.h \
 imexporter_be.h \
 job.h \
 job_be.h \
 provider.h \
 provider_be.h

libaqbanking_la_SOURCES=\
 account.c \
 banking.c \
 imexporter.c \
 job.c \
 provider.c

libaqbanking_la_LIBADD=@gwenhywfar_libs@ \
 jobs/libjobs.la \
 types/libtypes.la

libaqbanking_la_LDFLAGS=\
  -version-info @AQBANKING_SO_CURRENT@:@AQBANKING_SO_REVISION@:@AQBANKING_SO_AGE@

# dll stuff
if IS_WINDOWS

# you only need to setup these variables here, the rest is done automatically
LIBRARY_NAME = $(PACKAGE)32_$(AQBANKING_SO_EFFECTIVE)
DLLLDLIBS = -L/mingw/lib $(gwenhywfar_libs)
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

endif
# IS_WINDOWS
