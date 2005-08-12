INCLUDES=-I$(top_builddir)/headers -I$(srcdir)/.. -I$(srcdir)/../prg -I$(srcdir)/../dialogs @all_includes@

#EXTRA_DIST=$(UI_FILES)

noinst_LTLIBRARIES=libwidgets.la

iheaderdir=${includedir}/kbanking
iheader_HEADERS=\
 kbaccountlist.h \
 kbjoblist.h \
 kbplugindescrlist.h

libwidgets_la_SOURCES=\
 kbaccountlist.cpp \
 kbjoblist.cpp \
 kbplugindescrlist.cpp

BUILT_SOURCES = $(nodist_libwidgets_la_SOURCES) $(nodist_iheader_HEADERS)
CLEANFILES = $(BUILT_SOURCES)

sources:
	for f in $(libwidgets_la_SOURCES); do \
	  echo $(subdir)/$$f >>$(top_srcdir)/qtsources; \
	done

%.moc.cpp: %.h
	@qt3_moc@ -o $@ $<


%.ui.cpp: %.ui
	$(qt3_uic) $< -impl $<.h -o $@
%.ui.h: %.ui
	$(qt3_uic) $< -o $@
SUFFIXES = .ui .ui.h .ui.cpp .moc.cpp


