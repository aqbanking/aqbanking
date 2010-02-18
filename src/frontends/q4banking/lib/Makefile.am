SUBDIRS=widgets dialogs views

INCLUDES = \
  -I$(top_builddir)/headers \
  -I$(srcdir)/dialogs -I$(srcdir)/widgets -I$(srcdir)/views \
  -Idialogs -Iwidgets -Iviews \
  $(gwenhywfar_includes) $(qt4_includes)

DEFS += -DBUILDING_Q4BANKING \
  -DQ4BANKING_PLUGINS=\"@q4banking_plugindir@\" \
  -DQ4BANKING_HELPDIR=\"@q4banking_helpdir@\" \
  -DQ4BHELP_BINARY_NAME=\"q4b-help@Q4BANKING_SO_EFFECTIVE@$(EXEEXT)\"

AM_CPPFLAGS= @visibility_cflags@


lib_LTLIBRARIES=libq4banking.la

iheaderdir = $(includedir)/q4banking
iheader_HEADERS=\
 api.h \
 banking.h \
 qbflagstaff.h \
 qbanking.h \
 qbgui.h \
 qbcfgmodule.h

#noinst_HEADERS =

libq4banking_la_SOURCES=\
 banking.cpp \
 qbflagstaff.cpp \
 qbanking.cpp \
 qbgui.cpp \
 qbcfgmodule.cpp

MOC_FILES=\
 qbflagstaff.moc

nodist_libq4banking_la_SOURCES=

BUILT_SOURCES = $(MOC_FILES) $(nodist_libq4banking_la_SOURCES) version.h
CLEANFILES = $(BUILT_SOURCES) i18nsources ressource.rc

libq4banking_la_LIBADD=\
 widgets/libwidgets.la \
 dialogs/libdialogs.la \
 views/libviews.la \
 $(i18n_libs) $(aqbanking_internal_libs) $(gwenhywfar_libs) $(qt4_libs) -lQt3Support $(GWENGUI_QT4_LIBS)

libq4banking_la_LDFLAGS = -no-undefined \
 -version-info @Q4BANKING_SO_CURRENT@:@Q4BANKING_SO_REVISION@:@Q4BANKING_SO_AGE@

# Add the linker flags for windows resource file
libq4banking_la_DEPENDENCIES = \
 widgets/libwidgets.la \
 dialogs/libdialogs.la \
 views/libviews.la \
 $(aqbanking_internal_libs)
# if IS_WINDOWS
if IS_WINDOWS
libq4banking_la_DEPENDENCIES += ressource.o
libq4banking_la_LDFLAGS += -Wl,ressource.o
ressource.o: ressource.rc
	$(RC) -o $@ $<
endif
# IS_WINDOWS
# End of linker flags for windows resource file

sources:
	for f in $(libq4banking_la_SOURCES); do \
	  echo $(subdir)/$$f >>$(top_srcdir)/qtsources; \
	done
	for d in $(SUBDIRS); do \
	  $(MAKE) -C $$d sources; \
	done ;

%.moc: %.h
	$(qt4_moc) -o $@ $<

# Build and link a test program to verify the linker flags
noinst_PROGRAMS = testlib checklib
testlib_SOURCES = testlib.cpp
testlib_LDADD = libq4banking.la $(aqbanking_internal_libs) $(gwenhywfar_libs) $(qt4_libs) -lQtCore -lQt3Support
checklib_SOURCES = checklib.cpp
checklib_LDADD = libq4banking.la $(aqbanking_internal_libs) $(gwenhywfar_libs) $(qt4_libs) -lQtCore -lQt3Support
TESTS = checklib
clean-local:
	-rm -rf testfolder

# For qt3 compatibility of qt4
DEFS += -DQT3_SUPPORT

built_sources: $(BUILT_SOURCES)
	for d in $(SUBDIRS); do \
	  $(MAKE) -C $$d built_sources; \
	done ;

