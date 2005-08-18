INCLUDES=-I$(top_builddir)/headers -I../widgets -I../dialogs -I.. @all_includes@

noinst_LTLIBRARIES=libviews.la

noinst_HEADERS=accountview.h
nodist_noinst_HEADERS = accountview.ui.h

libviews_la_SOURCES = accountview.cpp
nodist_libviews_la_SOURCES =\
  accountview.ui.cpp accountview.ui.moc.cpp accountview.moc.cpp

UI_FILES = accountview.ui

EXTRA_DIST = $(UI_FILES)

# Make sure the generated source files are generated before any
# further compiling.
BUILT_SOURCES = $(nodist_libviews_la_SOURCES)
CLEANFILES = $(nodist_libviews_la_SOURCES) $(nodist_noinst_HEADERS)


sources:
	for f in $(libviews_la_SOURCES) $(foreach uifile,$(UI_FILES),$(uifile).cpp); do \
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
	for A in $(libviews_la_SOURCES) $(noinst_HEADERS); do \
	 yes | $(QT3TO4) $$A; \
	done
	make workaround-missing-uic3-include

workaround-missing-uic3-include:
	for A in transactionview.ui; do \
	 make $$A.h; \
	 mv $$A.h tmp.h; \
	 echo -e '#include <qpixmap.h>\n' > $$A.h; \
	 cat tmp.h >> $$A.h; \
	 rm tmp.h; \
	done
# For qt3 compatibility of qt4
DEFS += -DQT_COMPAT

built_sources: $(BUILT_SOURCES)

