INCLUDES=-I$(top_builddir)/headers -I../logmanager -I.. @all_includes@

#UI_FILES=
#EXTRA_DIST= $(UI_FILES)

noinst_LTLIBRARIES=libwidgets.la

noinst_HEADERS=accountlist.h userlist.h
libwidgets_la_SOURCES = accountlist.cpp userlist.cpp

# Make sure the generated source files are generated before any
# further compiling.
BUILT_SOURCES = $(nodist_libwidgets_la_SOURCES) $(nodist_noinst_HEADERS)
CLEANFILES = $(nodist_libwidgets_la_SOURCES) $(nodist_noinst_HEADERS)


sources:
	for f in $(libwidgets_la_SOURCES); do \
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
	for A in $(libwidgets_la_SOURCES) $(noinst_HEADERS); do \
	 $(YES) | $(QT3TO4) $$A; \
	done
	rm -f $(BUILT_SOURCES)

# For qt3 compatibility of qt4
DEFS += -DQT3_SUPPORT

built_sources: $(BUILT_SOURCES)

