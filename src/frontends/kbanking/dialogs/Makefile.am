INCLUDES=-I$(top_builddir)/headers -I../widgets -I.. -I../prg @all_includes@

UI_FILES=\
  processwatcher.ui \
  settings.ui \
  mapaccount.ui \
  importer.ui \
  pickstartdate.ui \
  selectbank.ui \
  printdialog.ui

EXTRA_DIST = $(UI_FILES)

noinst_LTLIBRARIES=libdialogs.la

nodist_iheader_HEADERS=\
 mapaccount.ui.h \
 processwatcher.ui.h \
 settings.ui.h \
 importer.ui.h \
 pickstartdate.ui.h \
 selectbank.ui.h \
 printdialog.ui.h

iheaderdir=${includedir}/kbanking
iheader_HEADERS=\
 processwatcher.h \
 settings.h \
 mapaccount.h \
 importer.h \
 pickstartdate.h \
 selectbank.h \
 printdialog.h
 

nodist_libdialogs_la_SOURCES=\
  settings.moc.cpp settings.ui.cpp settings.ui.moc.cpp \
  processwatcher.moc.cpp processwatcher.ui.cpp processwatcher.ui.moc.cpp \
  mapaccount.moc.cpp mapaccount.ui.cpp mapaccount.ui.moc.cpp \
  importer.moc.cpp importer.ui.cpp importer.ui.moc.cpp \
  pickstartdate.moc.cpp pickstartdate.ui.cpp pickstartdate.ui.moc.cpp \
  selectbank.moc.cpp selectbank.ui.cpp selectbank.ui.moc.cpp \
  printdialog.moc.cpp printdialog.ui.cpp printdialog.ui.moc.cpp


BUILT_SOURCES = $(nodist_libdialogs_la_SOURCES) $(nodist_iheader_HEADERS)
CLEANFILES = $(BUILT_SOURCES)

libdialogs_la_SOURCES=\
 processwatcher.cpp \
 settings.cpp \
 mapaccount.cpp \
 importer.cpp \
 pickstartdate.cpp \
 selectbank.cpp \
 printdialog.cpp


sources:
	for f in $(libdialogs_la_SOURCES) $(UI_FILES); do \
	  echo $(subdir)/$$f >>../i18nsources; \
	done


%.moc.cpp: %.h
	@qt3_moc@ -o $@ $<


%.ui.cpp: %.ui
	$(qt3_uic) $< -impl $<.h -o $@
%.ui.h: %.ui
	$(qt3_uic) $< -o $@
SUFFIXES = .ui .ui.h .ui.cpp .moc.cpp

