INCLUDES = \
  -I$(top_builddir)/headers \
  -I$(srcdir)/../widgets -I$(srcdir)/.. \
  -I../widgets -I.. \
  $(all_includes)


UI_FILES= \
  editctuser.ui \
  wizard.ui \
  getkeys.ui \
  mkkeys.ui \
  iniletter.ui

EXTRA_DIST= $(UI_FILES)

noinst_LTLIBRARIES=libwizard.la

nodist_noinst_HEADERS=\
  editctuser.ui.h \
  wizard.ui.h \
  getkeys.ui.h \
  mkkeys.ui.h \
  iniletter.ui.h

noinst_HEADERS=\
  editctuser.h \
  getkeys.h \
  mkkeys.h \
  iniletter.h \
  waction.h \
  winfo.h \
  wizard.h \
  a_edituser.h \
  a_getcert.h \
  a_getkeys.h \
  a_mkkeys.h \
  a_getsysid.h \
  a_wait.h \
  a_finished.h \
  a_getaccounts.h \
  w_pintan_new.h \
  w_ddv_import.h \
  w_rdh_import.h \
  w_rdh_new.h


nodist_libwizard_la_SOURCES=\
 editctuser.ui.cpp editctuser.ui.moc.cpp editctuser.moc.cpp \
 wizard.ui.cpp wizard.ui.moc.cpp wizard.moc.cpp \
 getkeys.ui.cpp getkeys.ui.moc.cpp getkeys.moc.cpp \
 mkkeys.ui.cpp mkkeys.ui.moc.cpp mkkeys.moc.cpp \
 iniletter.ui.cpp iniletter.ui.moc.cpp iniletter.moc.cpp

libwizard_la_SOURCES=\
  waction.cpp \
  winfo.cpp \
  wizard.cpp \
  editctuser.cpp \
  getkeys.cpp \
  mkkeys.cpp \
  iniletter.cpp \
  a_edituser.cpp \
  a_getcert.cpp \
  a_getkeys.cpp \
  a_mkkeys.cpp \
  a_getsysid.cpp \
  a_wait.cpp \
  a_finished.cpp \
  a_getaccounts.cpp \
  w_pintan_new.cpp \
  w_ddv_import.cpp \
  w_rdh_import.cpp \
  w_rdh_new.cpp


BUILT_SOURCES = $(nodist_libwizard_la_SOURCES)
CLEANFILES = $(nodist_libwizard_la_SOURCES) $(nodist_noinst_HEADERS)


sources:
	for f in $(libwizard_la_SOURCES) $(foreach uifile,$(UI_FILES),$(uifile).cpp); do \
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
	for A in $(libwizard_la_SOURCES) $(noinst_HEADERS); do \
	 $(YES) | $(QT3TO4) $$A; \
	done
	rm -f $(BUILT_SOURCES)
# For qt3 compatibility of qt4
DEFS += -DQT3_SUPPORT

built_sources: $(BUILT_SOURCES)

