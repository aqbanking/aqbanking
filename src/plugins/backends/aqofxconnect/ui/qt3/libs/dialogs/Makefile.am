INCLUDES=-I$(top_builddir)/headers -I../widgets -I.. @all_includes@

UI_FILES= editaccount.ui edituser.ui

EXTRA_DIST= $(UI_FILES)

noinst_LTLIBRARIES=libdialogs.la

noinst_HEADERS=editaccount.h edituser.h
nodist_noinst_HEADERS=editaccount.ui.h edituser.ui.h

nodist_libdialogs_la_SOURCES=\
 editaccount.ui.cpp editaccount.ui.moc.cpp editaccount.moc.cpp \
 edituser.ui.cpp edituser.ui.moc.cpp edituser.moc.cpp

libdialogs_la_SOURCES=editaccount.cpp edituser.cpp


BUILT_SOURCES = $(nodist_libdialogs_la_SOURCES)
CLEANFILES = $(nodist_libdialogs_la_SOURCES) $(nodist_noinst_HEADERS)


sources:
	for f in $(libdialogs_la_SOURCES); do \
	  echo $(subdir)/$$f >>$(top_srcdir)/qtsources; \
	done
	for d in $(SUBDIRS); do \
	  make -C $$d sources; \
	done


%.moc.cpp: %.h
	@qt3_moc@ -o $@ $<


# Build rules for the files that come from the .ui file
%.ui.cpp: %.ui
	$(qt3_uic) $< -impl $<.h -o $@
%.ui.h: %.ui
	$(qt3_uic) $< -o $@
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
