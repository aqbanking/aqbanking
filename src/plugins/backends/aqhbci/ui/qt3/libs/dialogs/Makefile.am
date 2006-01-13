SUBDIRS=wizard

INCLUDES = \
  -I$(top_builddir)/headers \
  -I$(srcdir)/.. -I$(srcdir)/wizard \
  -I.. -Iwizard \
  $(all_includes)


UI_FILES= \
 cfgtabpageuserhbci.ui \
 cfgtabpageaccounthbci.ui \
 selectmode.ui

EXTRA_DIST= $(UI_FILES)

noinst_LTLIBRARIES=libdialogs.la

nodist_noinst_HEADERS=\
 cfgtabpageuserhbci.ui.h \
 cfgtabpageaccounthbci.ui.h \
 selectmode.ui.h

noinst_HEADERS=\
 cfgtabpageuserhbci.h \
 cfgtabpageaccounthbci.h \
 selectmode.h \
 userwizard.h


nodist_libdialogs_la_SOURCES=\
 cfgtabpageuserhbci.ui.cpp cfgtabpageuserhbci.ui.moc.cpp cfgtabpageuserhbci.moc.cpp \
 cfgtabpageaccounthbci.ui.cpp cfgtabpageaccounthbci.ui.moc.cpp cfgtabpageaccounthbci.moc.cpp \
 selectmode.ui.cpp selectmode.ui.moc.cpp


libdialogs_la_SOURCES=\
 cfgtabpageuserhbci.cpp \
 cfgtabpageaccounthbci.cpp \
 selectmode.cpp \
 userwizard.cpp


libdialogs_la_LIBADD=wizard/libwizard.la

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
	for d in $(SUBDIRS); do \
	  make -C $$d qt4-port; \
	done
	for A in $(libdialogs_la_SOURCES) $(noinst_HEADERS); do \
	 $(YES) | $(QT3TO4) $$A; \
	done
	rm -f $(BUILT_SOURCES)
# For qt3 compatibility of qt4
DEFS += -DQT3_SUPPORT

built_sources: $(BUILT_SOURCES)
	for d in $(SUBDIRS); do \
	  make -C $$d built_sources; \
	done ;

