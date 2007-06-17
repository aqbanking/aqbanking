
INCLUDES = -I$(top_srcdir)/src/libs/aqbanking \
  -I$(top_builddir)/headers $(all_includes)
#EXTRA_DIST=README

bin_SCRIPTS=cbanking-config
M4dir = $(datadir)/aclocal
dist_M4_DATA=cbanking.m4

cbanking-config: cbanking-config.in
	echo "#! /bin/sh" >cbanking-config && \
	cat cbanking-config.in >>cbanking-config && \
	chmod a+x cbanking-config

CLEANFILES=cbanking-config cbanking-config.in

lib_LTLIBRARIES=libcbanking.la

iheaderdir=${includedir}/cbanking
iheader_HEADERS=cbanking.h

noinst_HEADERS=\
 cbanking_l.h \
 cbanking_p.h \
 progress_l.h \
 progress_p.h

libcbanking_la_SOURCES=\
 cbanking.c \
 progress.c

libcbanking_la_LIBADD = \
  $(aqbanking_internal_libs) $(gwenhywfar_libs)
libcbanking_la_LDFLAGS = -no-undefined $(STRIPALL) \
  -version-info @AQBANKING_SO_CURRENT@:@AQBANKING_SO_REVISION@:@AQBANKING_SO_AGE@

# Add the linker flags for windows resource file
libcbanking_la_DEPENDENCIES = \
 $(aqbanking_internal_libs)
# if IS_WINDOWS
if IS_WINDOWS
libcbanking_la_DEPENDENCIES += ressource.o
libcbanking_la_LDFLAGS += -Wl,ressource.o
ressource.o: ressource.rc
	$(RC) -o $@ $<
if USE_I18N
libcbanking_la_LIBADD += -liconv
endif
endif
# IS_WINDOWS
# End of linker flags for windows resource file

sources:
	for f in $(libcbanking_la_SOURCES); do \
	  echo $(subdir)/$$f >>$(top_srcdir)/i18nsources; \
	done


# Build and link a test program to verify the linker flags
noinst_PROGRAMS = testlib
testlib_SOURCES = testlib.c
testlib_LDADD = libcbanking.la $(aqbanking_internal_libs)
TESTS = testlib

built_sources:

