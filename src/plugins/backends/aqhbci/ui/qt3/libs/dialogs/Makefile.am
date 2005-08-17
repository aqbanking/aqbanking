INCLUDES = \
  -I$(top_builddir)/headers \
  -I$(srcdir)/../widgets -I$(srcdir)/.. \
  $(all_includes)


UI_FILES= \
  wizard.ui \
  selectcontext.ui \
  hbcisettings.ui \
  editaccount.ui \
  edituser.ui \
  editcustomer.ui \
  versionpicker.ui

EXTRA_DIST= $(UI_FILES)

noinst_LTLIBRARIES=libdialogs.la

nodist_noinst_HEADERS=\
  wizard.ui.h \
  selectcontext.ui.h \
  hbcisettings.ui.h \
  editaccount.ui.h \
  edituser.ui.h \
  editcustomer.ui.h \
  versionpicker.ui.h

noinst_HEADERS=\
  wizard.h \
  selectcontext.h \
  hbcisettings.h \
  editaccount.h \
  edituser.h \
  editcustomer.h \
  versionpicker.h


nodist_libdialogs_la_SOURCES=\
 wizard.ui.cpp wizard.ui.moc.cpp wizard.moc.cpp \
 selectcontext.ui.cpp selectcontext.ui.moc.cpp selectcontext.moc.cpp \
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
  wizard.cpp \
  pageacclist.cpp \
  pagecheckcard.cpp \
  pagecheckfile.cpp \
  pagecreatekeys.cpp \
  pageimport.cpp \
  pageiniletter.cpp \
  pagemedium.cpp \
  pageselectfile.cpp \
  pagesendkeys.cpp \
  pageuserdata.cpp \
  pageservercert.cpp \
  pageserverkeys.cpp \
  pageservertest.cpp \
  pagesummary1.cpp \
  pageinitmode.cpp \
  pagesysid.cpp \
  pageverifykeys.cpp \
  selectcontext.cpp \
  versionpicker.cpp


BUILT_SOURCES = $(nodist_libdialogs_la_SOURCES)
CLEANFILES = $(nodist_libdialogs_la_SOURCES) $(nodist_noinst_HEADERS)


sources:
	for f in $(libdialogs_la_SOURCES) $(nodist_libdialogs_la_SOURCES); do \
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

