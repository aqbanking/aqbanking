INCLUDES = \
  -I$(top_builddir)/headers \
  -I$(srcdir)/../widgets -I$(srcdir)/.. \
  $(all_includes)


UI_FILES= \
  editctuser.ui \
  wizard.ui

EXTRA_DIST= $(UI_FILES)

noinst_LTLIBRARIES=libwizard.la

nodist_noinst_HEADERS=\
  editctuser.ui.h \
  wizard.ui.h

noinst_HEADERS=\
  editctuser.h \
  waction.h \
  winfo.h \
  wizard.h \
  a_edituser.h \
  a_getcert.h \
  a_getsysid.h \
  a_finished.h \
  a_getaccounts.h \
  w_pintan_new.h \
  w_ddv_import.h \
  w_rdh_import.h


nodist_libwizard_la_SOURCES=\
 editctuser.ui.cpp editctuser.ui.moc.cpp editctuser.moc.cpp \
 wizard.ui.cpp wizard.ui.moc.cpp wizard.moc.cpp

libwizard_la_SOURCES=\
  waction.cpp \
  winfo.cpp \
  wizard.cpp \
  editctuser.cpp \
  a_edituser.cpp \
  a_getcert.cpp \
  a_getsysid.cpp \
  a_finished.cpp \
  a_getaccounts.cpp \
  w_pintan_new.cpp \
  w_ddv_import.cpp \
  w_rdh_import.cpp


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

