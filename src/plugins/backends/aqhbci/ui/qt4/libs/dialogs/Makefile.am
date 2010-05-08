SUBDIRS=wizard

INCLUDES = $(gwenhywfar_includes) $(qt4_includes) \
  -I$(aqbanking_symlinkdir) \
  -I$(srcdir)/.. -I$(srcdir)/wizard \
  $(GWENGUI_QT4_CFLAGS) \
  -I.. -Iwizard


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


nodist_libdialogs_la_SOURCES=


libdialogs_la_SOURCES=\
 cfgtabpageuserhbci.cpp \
 cfgtabpageaccounthbci.cpp \
 selectmode.cpp \
 userwizard.cpp

MOC_FILES=\
 cfgtabpageaccounthbci.moc \
 cfgtabpageaccounthbci.ui.moc \
 cfgtabpageuserhbci.moc


libdialogs_la_LIBADD=wizard/libwizard.la


BUILT_SOURCES = $(nodist_libdialogs_la_SOURCES) $(nodist_noinst_HEADERS) $(MOC_FILES)
CLEANFILES = $(BUILT_SOURCES)


sources:
	for f in $(libdialogs_la_SOURCES) $(foreach uifile,$(UI_FILES),$(uifile).h); do \
	  echo $(subdir)/$$f >>$(top_srcdir)/qtsources; \
	done
	for d in $(SUBDIRS); do \
	  $(MAKE) -C $$d sources; \
	done


%.moc: %.h
	$(qt4_moc) -o $@ $<


%.ui.h: %.ui
	$(qt4_uic) $< -o $@

SUFFIXES = .ui .ui.h .moc.cpp

# For qt3 compatibility of qt4
DEFS += -DQT3_SUPPORT

built_sources: $(BUILT_SOURCES)
	for d in $(SUBDIRS); do \
	  $(MAKE) -C $$d built_sources; \
	done ;

