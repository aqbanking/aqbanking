INCLUDES=-I$(top_builddir)/headers -I.. @all_includes@

UI_FILES=\
 cfgtabpageaccountofx.ui \
 cfgtabpageuserofx.ui

EXTRA_DIST= $(UI_FILES)

noinst_LTLIBRARIES=libdialogs.la

noinst_HEADERS=\
 cfgtabpageaccountofx.h \
 cfgtabpageuserofx.h
 
nodist_noinst_HEADERS=\
 cfgtabpageaccountofx.ui.h \
 cfgtabpageuserofx.ui.h

nodist_libdialogs_la_SOURCES=\
 cfgtabpageaccountofx.ui.cpp \
 cfgtabpageaccountofx.ui.moc.cpp \
 cfgtabpageuserofx.ui.cpp \
 cfgtabpageuserofx.ui.moc.cpp \
 cfgtabpageuserofx.moc.cpp

libdialogs_la_SOURCES=\
 cfgtabpageaccountofx.cpp \
 cfgtabpageuserofx.cpp


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
