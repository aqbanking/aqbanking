INCLUDES = \
  -I$(top_builddir)/headers \
  -I$(srcdir)/../widgets -I$(srcdir)/.. -I$(srcdir)/../prg \
  $(all_includes)

UI_FILES=\
  qbprocesswatcher.ui \
  qbsettings.ui \
  qbmapaccount.ui \
  qbimporter.ui \
  qbpickstartdate.ui \
  qbselectbank.ui \
  qbselectfromlist.ui \
  qbprintdialog.ui

EXTRA_DIST = $(UI_FILES)

noinst_LTLIBRARIES=libdialogs.la

nodist_iheader_HEADERS=\
 qbmapaccount.ui.h \
 qbprocesswatcher.ui.h \
 qbsettings.ui.h \
 qbimporter.ui.h \
 qbpickstartdate.ui.h \
 qbselectbank.ui.h \
 qbselectfromlist.ui.h \
 qbprintdialog.ui.h

iheaderdir=${includedir}/qbanking
iheader_HEADERS=\
 qbprocesswatcher.h \
 qbsettings.h \
 qbmapaccount.h \
 qbimporter.h \
 qbpickstartdate.h \
 qbselectbank.h \
 qbselectfromlist.h \
 qbprintdialog.h

nodist_libdialogs_la_SOURCES=\
  qbsettings.moc.cpp qbsettings.ui.cpp qbsettings.ui.moc.cpp \
  qbprocesswatcher.moc.cpp qbprocesswatcher.ui.cpp qbprocesswatcher.ui.moc.cpp \
  qbmapaccount.moc.cpp qbmapaccount.ui.cpp qbmapaccount.ui.moc.cpp \
  qbimporter.moc.cpp qbimporter.ui.cpp qbimporter.ui.moc.cpp \
  qbpickstartdate.moc.cpp qbpickstartdate.ui.cpp qbpickstartdate.ui.moc.cpp \
  qbselectbank.moc.cpp qbselectbank.ui.cpp qbselectbank.ui.moc.cpp \
  qbselectfromlist.moc.cpp qbselectfromlist.ui.cpp qbselectfromlist.ui.moc.cpp \
  qbprintdialog.moc.cpp qbprintdialog.ui.cpp qbprintdialog.ui.moc.cpp


BUILT_SOURCES = $(nodist_libdialogs_la_SOURCES) $(nodist_iheader_HEADERS)
CLEANFILES = $(BUILT_SOURCES)

libdialogs_la_SOURCES=\
 qbprocesswatcher.cpp \
 qbsettings.cpp \
 qbmapaccount.cpp \
 qbimporter.cpp \
 qbpickstartdate.cpp \
 qbselectbank.cpp \
 qbselectfromlist.cpp \
 qbprintdialog.cpp


sources:
	for f in $(libdialogs_la_SOURCES) $(UI_FILES); do \
	  echo $(subdir)/$$f >>$(top_srcdir)/qtsources; \
	done


%.moc.cpp: %.h
	@qt3_moc@ -o $@ $<


%.ui.cpp: %.ui
	$(qt3_uic) $< -impl $<.h -o $@
%.ui.h: %.ui
	$(qt3_uic) $< -o $@
SUFFIXES = .ui .ui.h .ui.cpp .moc.cpp

