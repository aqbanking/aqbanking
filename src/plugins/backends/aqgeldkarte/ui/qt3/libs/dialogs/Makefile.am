
INCLUDES = \
  -I$(top_builddir)/headers \
  -I$(srcdir)/../widgets -I$(srcdir)/.. \
  -I../widgets -I.. \
  $(all_includes)


UI_FILES= \
 cfgtabpagk.ui

EXTRA_DIST= $(UI_FILES)

noinst_LTLIBRARIES=libdialogs.la

nodist_noinst_HEADERS=\
 cfgtabpagk.ui.h

noinst_HEADERS=\
 cfgtabpagk.h


nodist_libdialogs_la_SOURCES=\
 cfgtabpagk.ui.cpp cfgtabpagk.ui.moc.cpp cfgtabpagk.moc.cpp


libdialogs_la_SOURCES=\
 cfgtabpagk.cpp


BUILT_SOURCES = $(nodist_libdialogs_la_SOURCES)
CLEANFILES = $(nodist_libdialogs_la_SOURCES) $(nodist_noinst_HEADERS)


sources:
	for f in $(libdialogs_la_SOURCES) $(foreach uifile,$(UI_FILES),$(uifile).cpp); do \
	  echo $(subdir)/$$f >>$(top_srcdir)/qtsources; \
	done
	for d in $(SUBDIRS); do \
	  $(MAKE) -C $$d sources; \
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
	for d in $(SUBDIRS); do \
	  $(MAKE) -C $$d qt4-port; \
	done
	for A in $(libdialogs_la_SOURCES) $(noinst_HEADERS); do \
	 $(YES) | $(QT3TO4) $$A; \
	done
	rm -f $(BUILT_SOURCES)
# For qt3 compatibility of qt4
DEFS += -DQT3_SUPPORT

built_sources: $(BUILT_SOURCES)
	for d in $(SUBDIRS); do \
	  $(MAKE) -C $$d built_sources; \
	done ;

