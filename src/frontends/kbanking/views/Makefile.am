INCLUDES = \
  -I$(top_builddir)/headers \
  -I$(srcdir)/../widgets -I$(srcdir)/../dialogs \
  -I$(srcdir)/../prg -I$(srcdir)/.. \
  $(all_includes)

noinst_LTLIBRARIES=libviews.la

iheaderdir=${includedir}/kbanking
iheader_HEADERS=\
 jobview.h

nodist_iheader_HEADERS= \
 jobview.ui.h

libviews_la_SOURCES=\
 jobview.cpp

nodist_libviews_la_SOURCES=\
  jobview.ui.cpp jobview.ui.moc.cpp \
  jobview.moc.cpp

UI_FILES = jobview.ui

EXTRA_DIST = $(UI_FILES)

# Make sure the generated source files are generated before any
# further compiling.
BUILT_SOURCES = $(nodist_libviews_la_SOURCES) $(nodist_iheader_HEADERS)
CLEANFILES = $(BUILT_SOURCES)

sources:
	for f in $(libviews_la_SOURCES) $(UI_FILES); do \
	  echo $(subdir)/$$f >>../i18nsources; \
	done


%.moc.cpp: %.h
	@qt3_moc@ -o $@ $<

# Build rules for the files that come from the .ui file
%.ui.cpp: %.ui
	$(qt3_uic) $< -impl $<.h -o $@
%.ui.h: %.ui
	$(qt3_uic) $< -o $@
SUFFIXES = .ui .ui.h .ui.cpp .moc.cpp

