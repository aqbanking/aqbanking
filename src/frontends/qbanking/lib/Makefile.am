SUBDIRS=widgets dialogs views gui

INCLUDES = \
  -I$(top_builddir)/headers \
  -I$(srcdir)/dialogs -I$(srcdir)/widgets -I$(srcdir)/views \
  -Idialogs -Iwidgets -Iviews \
  -I$(top_srcdir)/src/libs/aqbanking++ \
  $(all_includes)

DEFS += -DBUILDING_QBANKING \
  -DQBANKING_PLUGINS=\"@qbanking_plugindir@\" \
  -DQBANKING_HELPDIR=\"@qbanking_helpdir@\" \
  -DQBHELP_BINARY_NAME=\"qb-help@QBANKING_SO_EFFECTIVE@$(EXEEXT)\"

AM_CPPFLAGS= @visibility_cflags@


lib_LTLIBRARIES=libqbanking.la

iheaderdir = $(includedir)/qbanking
iheader_HEADERS=\
 api.h \
 banking.h \
 qbflagstaff.h \
 qbanking.h \
 qbgui.h \
 qbcfgmodule.h

#noinst_HEADERS =

libqbanking_la_SOURCES=\
 banking.cpp \
 qbflagstaff.cpp \
 qbanking.cpp \
 qbgui.cpp \
 qbcfgmodule.cpp

MOC_FILES=\
 qbflagstaff.moc

nodist_libqbanking_la_SOURCES=

BUILT_SOURCES = $(MOC_FILES) $(nodist_libqbanking_la_SOURCES) version.h
CLEANFILES = $(BUILT_SOURCES) i18nsources ressource.rc

libqbanking_la_LIBADD=\
 widgets/libwidgets.la \
 dialogs/libdialogs.la \
 views/libviews.la \
 gui/libqgui.la \
 $(aqbanking_internal_libs) $(gwenhywfar_libs) $(qt3_libs)

libqbanking_la_LDFLAGS = -no-undefined \
 -version-info @QBANKING_SO_CURRENT@:@QBANKING_SO_REVISION@:@QBANKING_SO_AGE@

# Add the linker flags for windows resource file
libqbanking_la_DEPENDENCIES = \
 widgets/libwidgets.la \
 dialogs/libdialogs.la \
 views/libviews.la \
 gui/libqgui.la \
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
	  $(MAKE) -C $$d sources; \
	done ;

%.moc: %.h
	$(qt3_moc) -o $@ $<

# Build and link a test program to verify the linker flags
noinst_PROGRAMS = testlib checklib
testlib_SOURCES = testlib.cpp
testlib_LDADD = libqbanking.la $(aqbanking_internal_libs)
checklib_SOURCES = checklib.cpp
checklib_LDADD = libqbanking.la $(aqbanking_internal_libs)
TESTS = checklib
clean-local:
	-rm -rf testfolder

# This section is for a qt4 environment
qt4-port:
	for A in $(libqbanking_la_SOURCES) $(HEADERS); do \
	 $(YES) | $(QT3TO4) $$A; \
	done
	for d in $(SUBDIRS); do \
	  $(MAKE) -C $$d qt4-port; \
	done
# For qt3 compatibility of qt4
DEFS += -DQT3_SUPPORT

built_sources: $(BUILT_SOURCES)
	for d in $(SUBDIRS); do \
	  $(MAKE) -C $$d built_sources; \
	done ;

