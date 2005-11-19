SUBDIRS=widgets dialogs views po

SUBPACKAGE=kbanking

INCLUDES = \
  -I$(top_builddir)/headers \
  -I$(srcdir)/dialogs -I$(srcdir)/widgets -I$(srcdir)/prg -I$(srcdir)/views \
  -Idialogs -Iwidgets -Iprg -Iviews \
  $(all_includes)

DEFS += -DPKGDATADIR=\"$(pkgdatadir)\"

lib_LTLIBRARIES=libkbanking.la

bin_SCRIPTS=kbanking-config
M4dir=${prefix}/share/aclocal
dist_M4_DATA=kbanking.m4

iheaderdir=${includedir}/kbanking
iheader_HEADERS=\
 kbflagstaff.h \
 kbanking_p.h \
 kbanking.h

libkbanking_la_SOURCES=\
 kbflagstaff.cpp \
 kbanking.cpp

nodist_libkbanking_la_SOURCES=\
 kbflagstaff.moc.cpp

kbanking-config: kbanking-config.in
	echo "#! /bin/sh" >kbanking-config && \
	cat kbanking-config.in >>kbanking-config && \
	chmod a+x kbanking-config

BUILT_SOURCES =$(nodist_libkbanking_la_SOURCES) version.h
CLEANFILES=$(BUILT_SOURCES) kbanking-config

libkbanking_la_LIBADD=\
 widgets/libwidgets.la \
 dialogs/libdialogs.la \
 views/libviews.la \
 -L../qbanking -lqbanking \
 $(aqbanking_internal_libs) \
 $(kde3_libs) -lkdeui -lkdecore -lkio $(gwenhywfar_libs)

libkbanking_la_LDFLAGS=\
 -version-info @KBANKING_SO_CURRENT@:@KBANKING_SO_REVISION@:@KBANKING_SO_AGE@


sources:
	for f in $(libkbanking_la_SOURCES); do \
	  echo $(subdir)/$$f >>$(top_srcdir)/qtsources; \
	done
	for d in $(SUBDIRS); do \
	  make -C $$d sources; \
	done ;


# Build and link a test program to verify the linker flags
noinst_PROGRAMS = testlib
testlib_SOURCES = testlib.cpp
testlib_LDADD = libkbanking.la


%.moc.cpp: %.h
	@qt3_moc@ -o $@ $<


clean-local:
	rm -f *.ui.{h.cpp} *.moc.* >/dev/null

clean-ui: 
	rm -f *.ui.{cpp,h}

built_sources: $(BUILT_SOURCES)
	for d in $(SUBDIRS); do \
	  make -C $$d built_sources; \
	done ;


