INCLUDES = \
  -I$(top_builddir)/headers \
  -I$(srcdir)/../widgets -I$(srcdir)/../dialogs \
  -I$(srcdir)/../prg -I$(srcdir)/.. \
  $(all_includes)

noinst_LTLIBRARIES=libviews.la

iheaderdir=${includedir}/qbanking
iheader_HEADERS=\
 qbjobview.h

nodist_iheader_HEADERS= \
 qbjobview.ui.h

libviews_la_SOURCES=\
 qbjobview.cpp

nodist_libviews_la_SOURCES=\
  qbjobview.ui.cpp qbjobview.ui.moc.cpp \
  qbjobview.moc.cpp

UI_FILES = qbjobview.ui

EXTRA_DIST = $(UI_FILES)

# Make sure the generated source files are generated before any
# further compiling.
BUILT_SOURCES = $(nodist_libviews_la_SOURCES) $(nodist_iheader_HEADERS)
CLEANFILES = $(BUILT_SOURCES)

sources:
	for f in $(libviews_la_SOURCES) $(UI_FILES); do \
	  echo $(subdir)/$$f >>$(top_srcdir)/qtsources; \
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
	for A in $(libviews_la_SOURCES) $(iheader_HEADERS); do \
	 $(YES) | $(QT3TO4) $$A; \
	done
# For qt3 compatibility of qt4
DEFS += -DQT3_SUPPORT
