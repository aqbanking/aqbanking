INCLUDES = \
  -I$(top_builddir)/headers \
  -I$(srcdir)/../widgets -I$(srcdir)/.. \
  $(all_includes)

DEFS += -DAQHBCI_DATAFOLDER=\"@aqhbci_data@\"

UI_FILES= \
  logmanager.ui

EXTRA_DIST= $(UI_FILES)

noinst_LTLIBRARIES=liblogmanager.la

nodist_noinst_HEADERS=\
  logmanager.ui.h

noinst_HEADERS=\
  logmanager.h \
  error.h \
  pointer.h \
  loganalyzer.h

nodist_liblogmanager_la_SOURCES=\
 logmanager.ui.cpp logmanager.ui.moc.cpp logmanager.moc.cpp

liblogmanager_la_SOURCES=\
  logmanager.cpp \
  error.cpp \
  loganalyzer.cpp

liblogmanager_la_LIBADD = \
  -L$(top_builddir)/src/plugins/backends/aqhbci/plugin $(aqhbci_internal_libs)

BUILT_SOURCES = $(nodist_liblogmanager_la_SOURCES)
CLEANFILES = $(nodist_liblogmanager_la_SOURCES) $(nodist_noinst_HEADERS)


sources:
	for f in $(liblogmanager_la_SOURCES); do \
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
	for A in $(liblogmanager_la_SOURCES) $(noinst_HEADERS); do \
	 $(YES) | $(QT3TO4) $$A; \
	done
	rm -f $(BUILT_SOURCES)
# For qt3 compatibility of qt4
DEFS += -DQT3_SUPPORT
