INCLUDES = \
  -I$(top_builddir)/headers \
  -I$(srcdir)/../widgets -I$(srcdir)/.. -I$(srcdir)/../prg \
  $(all_includes)

AM_CFLAGS=-DBUILDING_QBANKING @visibility_cflags@

UI_FILES=\
 qbprocesswatcher.ui \
 qbmapaccount.ui \
 qbimporter.ui \
 qbpickstartdate.ui \
 qbselectbank.ui \
 qbselectfromlist.ui \
 qbprintdialog.ui \
 qbcfgtabpageusergen.ui \
 qbcfgtabpageaccountgen.ui \
 qbcfgtabpageaccounts.ui \
 qbcfgtabpageusers.ui \
 qbcfgtabpagebackends.ui \
 qbselectbackend.ui

EXTRA_DIST = $(UI_FILES)

noinst_LTLIBRARIES=libdialogs.la

nodist_iheader_HEADERS=\
 qbmapaccount.ui.h \
 qbprocesswatcher.ui.h \
 qbimporter.ui.h \
 qbpickstartdate.ui.h \
 qbselectbank.ui.h \
 qbselectfromlist.ui.h \
 qbprintdialog.ui.h \
 qbcfgtabpageusergen.ui.h \
 qbcfgtabpageaccountgen.ui.h \
 qbcfgtabpageaccounts.ui.h \
 qbcfgtabpageusers.ui.h \
 qbcfgtabpagebackends.ui.h \
 qbselectbackend.ui.h
 

iheaderdir=${includedir}/qbanking
iheader_HEADERS=\
 qbprocesswatcher.h \
 qbmapaccount.h \
 qbimporter.h \
 qbpickstartdate.h \
 qbselectbank.h \
 qbselectfromlist.h \
 qbprintdialog.h \
 qbcfgtabpageusergen.h \
 qbcfgtabpageaccountgen.h \
 qbcfgtabpageaccounts.h \
 qbcfgtabpageusers.h \
 qbcfgtabpagebackends.h \
 qbedituser.h \
 qbeditaccount.h \
 qbcfgtabsettings.h \
 qbselectbackend.h

nodist_libdialogs_la_SOURCES=\
  qbprocesswatcher.moc.cpp qbprocesswatcher.ui.cpp qbprocesswatcher.ui.moc.cpp \
  qbmapaccount.moc.cpp qbmapaccount.ui.cpp qbmapaccount.ui.moc.cpp \
  qbimporter.moc.cpp qbimporter.ui.cpp qbimporter.ui.moc.cpp \
  qbpickstartdate.moc.cpp qbpickstartdate.ui.cpp qbpickstartdate.ui.moc.cpp \
  qbselectbank.moc.cpp qbselectbank.ui.cpp qbselectbank.ui.moc.cpp \
  qbselectfromlist.moc.cpp qbselectfromlist.ui.cpp qbselectfromlist.ui.moc.cpp \
  qbprintdialog.moc.cpp qbprintdialog.ui.cpp qbprintdialog.ui.moc.cpp \
  qbcfgtabpageusergen.moc.cpp qbcfgtabpageusergen.ui.cpp qbcfgtabpageusergen.ui.moc.cpp \
  qbcfgtabpageaccountgen.moc.cpp qbcfgtabpageaccountgen.ui.cpp qbcfgtabpageaccountgen.ui.moc.cpp \
  qbcfgtabpageaccounts.moc.cpp qbcfgtabpageaccounts.ui.cpp qbcfgtabpageaccounts.ui.moc.cpp \
  qbcfgtabpageusers.moc.cpp qbcfgtabpageusers.ui.cpp qbcfgtabpageusers.ui.moc.cpp \
  qbcfgtabpagebackends.moc.cpp qbcfgtabpagebackends.ui.cpp qbcfgtabpagebackends.ui.moc.cpp \
  qbcfgtabsettings.moc.cpp \
  qbselectbackend.moc.cpp qbselectbackend.ui.cpp qbselectbackend.ui.moc.cpp


BUILT_SOURCES = $(nodist_libdialogs_la_SOURCES) $(nodist_iheader_HEADERS)
CLEANFILES = $(BUILT_SOURCES)

libdialogs_la_SOURCES=\
 qbprocesswatcher.cpp \
 qbmapaccount.cpp \
 qbimporter.cpp \
 qbpickstartdate.cpp \
 qbselectbank.cpp \
 qbselectfromlist.cpp \
 qbprintdialog.cpp \
 qbcfgtabpageusergen.cpp \
 qbcfgtabpageaccountgen.cpp \
 qbcfgtabpageaccounts.cpp \
 qbcfgtabpageusers.cpp \
 qbcfgtabpagebackends.cpp \
 qbedituser.cpp \
 qbeditaccount.cpp \
 qbcfgtabsettings.cpp \
 qbselectbackend.cpp


sources:
	for f in $(libdialogs_la_SOURCES) $(foreach uifile,$(UI_FILES),$(uifile).cpp); do \
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
	for A in $(libdialogs_la_SOURCES) $(iheader_HEADERS); do \
	 $(YES) | $(QT3TO4) $$A; \
	done
# For qt3 compatibility of qt4
DEFS += -DQT3_SUPPORT

built_sources: $(BUILT_SOURCES)
	list='$(SUBDIRS)'; for d in $$list; do \
	  make -C $$d built_sources; \
	done ;
