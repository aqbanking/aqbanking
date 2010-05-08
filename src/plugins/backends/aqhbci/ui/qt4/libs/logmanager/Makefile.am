INCLUDES = \
  -I$(aqbanking_symlinkdir) \
  -I$(srcdir)/../widgets -I$(srcdir)/.. \
  -I../widgets -I.. \
  $(GWENGUI_QT4_CFLAGS) \
  $(gwenhywfar_includes) $(qt4_includes)

DEFS += -DAQHBCI_DATAFOLDER=\"$(aqbanking_pkgdatadir)/backends/aqhbci\"

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

nodist_liblogmanager_la_SOURCES=

liblogmanager_la_SOURCES=\
  logmanager.cpp \
  error.cpp \
  loganalyzer.cpp

liblogmanager_la_LIBADD = $(q4banking_internal_libs) $(aqhbci_internal_libs)

MOC_FILES=\
  logmanager.moc


BUILT_SOURCES = $(nodist_liblogmanager_la_SOURCES) $(nodist_noinst_HEADERS) $(MOC_FILES)
CLEANFILES = $(BUILT_SOURCES)


sources:
	for f in $(liblogmanager_la_SOURCES) $(foreach uifile,$(UI_FILES),$(uifile).h); do \
	  echo $(subdir)/$$f >>$(top_srcdir)/qtsources; \
	done
	for d in $(SUBDIRS); do \
	  $(MAKE) -C $$d sources; \
	done


%.moc: %.h
	$(qt4_moc) -o $@ $<


# Build rules for the files that come from the .ui file
%.ui.h: %.ui
	$(qt4_uic) $< -o $@

SUFFIXES = .ui .ui.h .moc.cpp

# For qt3 compatibility of qt4
DEFS += -DQT3_SUPPORT

built_sources: $(BUILT_SOURCES)

