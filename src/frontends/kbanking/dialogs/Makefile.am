INCLUDES=-I$(top_builddir)/headers -I$(srcdir)/../widgets -I$(srcdir)/.. -I$(srcdir)/../prg @all_includes@

UI_FILES=\
  settings.ui

EXTRA_DIST = $(UI_FILES)

noinst_LTLIBRARIES=libdialogs.la

nodist_iheader_HEADERS=\
 settings.ui.h

iheaderdir=${includedir}/kbanking
iheader_HEADERS=\
 settings.h 

nodist_libdialogs_la_SOURCES=\
  settings.moc.cpp settings.ui.cpp settings.ui.moc.cpp


BUILT_SOURCES = $(nodist_libdialogs_la_SOURCES) $(nodist_iheader_HEADERS)
CLEANFILES = $(BUILT_SOURCES)

libdialogs_la_SOURCES=\
 settings.cpp


sources:
	for f in $(libdialogs_la_SOURCES) $(UI_FILES); do \
	  echo $(subdir)/$$f >>$(top_srcdir)/qtsources; \
	done


%.moc.cpp: %.h
	@qt3_moc@ -o $@ $<


%.ui.cpp: %.ui
	$(qt3_uic) $< -impl $<.h -o $@
%.ui.h: %.ui
	$(qt3_uic) $< -o $@
SUFFIXES = .ui .ui.h .ui.cpp .moc.cpp

