SUBDIRS=jobs types

INCLUDES=-I${top_srcdir} -Itypes -Ijobs @all_includes@

lib_LTLIBRARIES=libaqbanking.la

# We define those here because within the configure script the variables 
# $(datadir) et al are not available.
localedir=$(datadir)/locale
DEFS= @DEFS@ \
 -DLOCALEDIR=\"$(localedir)\" \
 -DAQBANKING_PLUGINS=\"@aqbanking_plugins@\" \
 -DDATADIR=\"$(aqbanking_data)\"

noinst_HEADERS=\
 account_l.h \
 account_p.h \
 banking_l.h \
 banking_p.h \
 bankinfoplugin_l.h \
 bankinfoplugin_p.h \
 country_l.h \
 country_p.h \
 i18n_l.h \
 imexporter_l.h \
 imexporter_p.h \
 job_l.h \
 job_p.h \
 provider_l.h \
 provider_p.h


iheaderdir=${includedir}/aqbanking
iheader_HEADERS=\
 account.h \
 account_be.h \
 banking.h \
 banking_be.h \
 bankinfoplugin.h \
 bankinfoplugin_be.h \
 country.h \
 error.h \
 imexporter.h \
 imexporter_be.h \
 job.h \
 job_be.h \
 provider.h \
 provider_be.h

libaqbanking_la_SOURCES=\
 account.c \
 bankinfoplugin.c \
 banking.c \
 country.c \
 imexporter.c \
 job.c \
 provider.c

libaqbanking_la_LIBADD=@gwenhywfar_libs@ \
 jobs/libjobs.la \
 types/libtypes.la

libaqbanking_la_LDFLAGS=\
  -version-info @AQBANKING_SO_CURRENT@:@AQBANKING_SO_REVISION@:@AQBANKING_SO_AGE@


sources:
	for f in $(libaqbanking_la_SOURCES); do \
	  echo $(subdir)/$$f >>$(top_srcdir)/i18nsources; \
	done
	for d in $(SUBDIRS); do \
	  make -C $$d sources; \
	done


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
