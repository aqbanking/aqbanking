INCLUDES = \
  -I$(top_builddir)/headers \
  -I$(srcdir)/.. -I$(srcdir)/../prg -I$(srcdir)/../dialogs \
  $(all_includes)

UI_FILES= \
 qbprogress.ui \
 qbsimplebox.ui

EXTRA_DIST=$(UI_FILES)

noinst_LTLIBRARIES=libwidgets.la

nodist_iheader_HEADERS=\
 qbprogress.ui.h \
 qbsimplebox.ui.h

iheaderdir=${includedir}/qbanking
iheader_HEADERS=\
 qbsimplebox.h \
 qbaccountlist.h \
 qbinputbox.h \
 qbjoblist.h \
 qbplugindescrlist.h \
 qbprogress.h

nodist_libwidgets_la_SOURCES=\
 qbprogress.ui.cpp qbprogress.ui.moc.cpp \
 qbsimplebox.ui.cpp qbsimplebox.ui.moc.cpp \
 qbinputbox.moc.cpp \
 qbprogress.moc.cpp \
 qbsimplebox.moc.cpp


libwidgets_la_SOURCES=\
 qbaccountlist.cpp \
 qbjoblist.cpp \
 qbinputbox.cpp \
 qbplugindescrlist.cpp \
 qbprogress.cpp \
 qbsimplebox.cpp

BUILT_SOURCES = $(nodist_libwidgets_la_SOURCES) $(nodist_iheader_HEADERS)
CLEANFILES = $(BUILT_SOURCES)

sources:
	for f in $(libwidgets_la_SOURCES) $(UI_FILES); do \
	  echo $(subdir)/$$f >>$(top_srcdir)/qtsources; \
	done

%.moc.cpp: %.h
	@qt3_moc@ -o $@ $<


%.ui.cpp: %.ui
	$(UIC3) $< -impl $<.h -o $@
%.ui.h: %.ui
	$(UIC3) $< -o $@
SUFFIXES = .ui .ui.h .ui.cpp .moc.cpp


# This section is for a qt4 environment
qt4-port:
	for A in $(libwidgets_la_SOURCES) $(iheader_HEADERS); do \
	 $(YES) | $(QT3TO4) $$A; \
	done
# For qt3 compatibility of qt4
DEFS += -DQT3_SUPPORT
