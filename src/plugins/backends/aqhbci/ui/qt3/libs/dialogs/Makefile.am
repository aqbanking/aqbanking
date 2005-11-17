SUBDIRS=wizard

INCLUDES = \
  -I$(top_builddir)/headers \
  -I$(srcdir)/../widgets -I$(srcdir)/.. -Iwizard \
  $(all_includes)


UI_FILES= \
  selectcontext.ui \
  selectmode.ui \
  hbcisettings.ui \
  editaccount.ui \
  edituser.ui \
  editcustomer.ui \
  versionpicker.ui

EXTRA_DIST= $(UI_FILES)

noinst_LTLIBRARIES=libdialogs.la

nodist_noinst_HEADERS=\
  selectcontext.ui.h \
  selectmode.ui.h \
  hbcisettings.ui.h \
  editaccount.ui.h \
  edituser.ui.h \
  editcustomer.ui.h \
  versionpicker.ui.h

noinst_HEADERS=\
  selectcontext.h \
  selectmode.h \
  hbcisettings.h \
  editaccount.h \
  edituser.h \
  editcustomer.h \
  versionpicker.h \
  userwizard.h


nodist_libdialogs_la_SOURCES=\
 selectcontext.ui.cpp selectcontext.ui.moc.cpp selectcontext.moc.cpp \
 selectmode.ui.cpp selectmode.ui.moc.cpp \
 hbcisettings.ui.cpp hbcisettings.ui.moc.cpp hbcisettings.moc.cpp \
 editaccount.ui.cpp editaccount.ui.moc.cpp editaccount.moc.cpp \
 edituser.ui.cpp edituser.ui.moc.cpp edituser.moc.cpp \
 editcustomer.ui.cpp editcustomer.ui.moc.cpp editcustomer.moc.cpp \
 versionpicker.ui.cpp versionpicker.ui.moc.cpp versionpicker.moc.cpp

libdialogs_la_SOURCES=\
  editaccount.cpp \
  edituser.cpp \
  editcustomer.cpp \
  hbcisettings.cpp \
  selectcontext.cpp \
  selectmode.cpp \
  versionpicker.cpp \
  userwizard.cpp

libdialogs_la_LIBADD=wizard/libwizard.la

BUILT_SOURCES = $(nodist_libdialogs_la_SOURCES)
CLEANFILES = $(nodist_libdialogs_la_SOURCES) $(nodist_noinst_HEADERS)


sources:
	for f in $(libdialogs_la_SOURCES) $(foreach uifile,$(UI_FILES),$(uifile).cpp); do \
	  echo $(subdir)/$$f >>$(top_srcdir)/qtsources; \
	done
	for d in $(SUBDIRS); do \
	  make -C $$d sources; \
	done


%.moc.cpp: %.h
	@qt3_moc@ -o $@ $<


# Build rules for the files that come from the .ui file
%.ui.cpp: %.ui
	$(UIC3) $< -impl $<.h -o $@
%.ui.h: %.ui
	$(UIC3) $< -o $@
SUFFIXES = .ui .ui.h .ui.cpp .moc.cpp

# This section is for a qt4 environment
qt4-port:
	for A in $(libdialogs_la_SOURCES) $(noinst_HEADERS); do \
	 $(YES) | $(QT3TO4) $$A; \
	done
	rm -f $(BUILT_SOURCES)
# For qt3 compatibility of qt4
DEFS += -DQT3_SUPPORT

built_sources: $(BUILT_SOURCES)

