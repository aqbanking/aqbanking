SUBDIRS=widgets dialogs views

INCLUDES = \
  -I$(top_builddir)/headers \
  -I$(srcdir)/dialogs -I$(srcdir)/widgets -I$(srcdir)/views \
  -Idialogs -Iwidgets -Iviews \
  $(all_includes)

DEFS += -DBUILDING_QBANKING \
  -DPKGDATADIR=\"$(pkgdatadir)\" \
  -DQBANKING_PLUGINS=\"@qbanking_plugindir@\" \
  -DQBANKING_HELPDIR=\"@qbanking_helpdir@\"

AM_CPPFLAGS= @visibility_cflags@


lib_LTLIBRARIES=libqbanking.la

bin_SCRIPTS=qbanking-config
M4dir = $(datadir)/aclocal
dist_M4_DATA=qbanking.m4

iheaderdir = $(includedir)/qbanking
iheader_HEADERS=\
 qbflagstaff.h \
 qbanking.h \
 qbwaitcallback.h \
 qbwcb_fast.h \
 qbwcb_simple.h \
 qbwcb_progress.h \
 qbcfgmodule.h \
 banking.h

noinst_HEADERS = \
 qbanking_p.h \
 banking_p.h

libqbanking_la_SOURCES=\
 qbflagstaff.cpp \
 qbanking.cpp \
 qbwaitcallback.cpp \
 qbwcb_fast.cpp \
 qbwcb_simple.cpp \
 qbwcb_progress.cpp \
 qbcfgmodule.cpp \
 banking.cpp

MOC_FILES=\
 qbflagstaff.moc

nodist_libqbanking_la_SOURCES=

qbanking-config: qbanking-config.in
	echo "#! /bin/sh" >qbanking-config && \
	cat qbanking-config.in >>qbanking-config && \
	chmod a+x qbanking-config

BUILT_SOURCES = $(MOC_FILES) $(nodist_libqbanking_la_SOURCES) version.h
CLEANFILES = $(BUILT_SOURCES) i18nsources qbanking-config qbanking-config.in ressource.rc

libqbanking_la_LIBADD=\
 widgets/libwidgets.la \
 dialogs/libdialogs.la \
 views/libviews.la \
 $(aqbanking_internal_libs) $(gwenhywfar_libs) $(qt3_libs)

libqbanking_la_LDFLAGS = -no-undefined \
 -version-info @QBANKING_SO_CURRENT@:@QBANKING_SO_REVISION@:@QBANKING_SO_AGE@

# Add the linker flags for windows resource file
libqbanking_la_DEPENDENCIES = \
 widgets/libwidgets.la \
 dialogs/libdialogs.la \
 views/libviews.la \
 $(aqbanking_internal_libs)
# if IS_WINDOWS
if IS_WINDOWS
libqbanking_la_DEPENDENCIES += ressource.o
libqbanking_la_LDFLAGS += -Wl,ressource.o
ressource.o: ressource.rc
	$(RC) -o $@ $<
endif
# IS_WINDOWS
# End of linker flags for windows resource file

sources:
	for f in $(libqbanking_la_SOURCES); do \
	  echo $(subdir)/$$f >>$(top_srcdir)/qtsources; \
	done
	for d in $(SUBDIRS); do \
	  make -C $$d sources; \
	done ;

%.moc: %.h
	$(qt3_moc) -o $@ $<

# Build and link a test program to verify the linker flags
noinst_PROGRAMS = testlib
testlib_SOURCES = testlib.cpp
testlib_LDADD = libqbanking.la


# This section is for a qt4 environment
qt4-port:
	for A in $(libqbanking_la_SOURCES) $(HEADERS); do \
	 $(YES) | $(QT3TO4) $$A; \
	done
	for d in $(SUBDIRS); do \
	  make -C $$d qt4-port; \
	done
# For qt3 compatibility of qt4
DEFS += -DQT3_SUPPORT

built_sources: $(BUILT_SOURCES)
	for d in $(SUBDIRS); do \
	  make -C $$d built_sources; \
	done ;

