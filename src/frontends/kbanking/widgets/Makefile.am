INCLUDES=-I$(top_builddir)/headers -I$(srcdir)/.. -I$(srcdir)/../prg -I$(srcdir)/../dialogs @all_includes@

UI_FILES= \
 progress.ui \
 simplebox.ui

EXTRA_DIST=$(UI_FILES)

noinst_LTLIBRARIES=libwidgets.la

nodist_iheader_HEADERS=\
 progress.ui.h \
 simplebox.ui.h

iheaderdir=${includedir}/kbanking
iheader_HEADERS=\
 simplebox.h \
 accountlist.h \
 inputbox.h \
 joblist.h \
 plugindescrlist.h \
 progress.h

nodist_libwidgets_la_SOURCES=\
 progress.ui.cpp progress.ui.moc.cpp \
 simplebox.ui.cpp simplebox.ui.moc.cpp \
 inputbox.moc.cpp \
 progress.moc.cpp \
 simplebox.moc.cpp


libwidgets_la_SOURCES=\
 accountlist.cpp \
 joblist.cpp \
 inputbox.cpp \
 plugindescrlist.cpp \
 progress.cpp \
 simplebox.cpp

BUILT_SOURCES = $(nodist_libwidgets_la_SOURCES) $(nodist_iheader_HEADERS)
CLEANFILES = $(BUILT_SOURCES)

sources:
	for f in $(libwidgets_la_SOURCES) $(UI_FILES); do \
	  echo $(subdir)/$$f >>../i18nsources; \
	done

%.moc.cpp: %.h
	@qt3_moc@ -o $@ $<


%.ui.cpp: %.ui
	$(qt3_uic) $< -impl $<.h -o $@
%.ui.h: %.ui
	$(qt3_uic) $< -o $@
SUFFIXES = .ui .ui.h .ui.cpp .moc.cpp


