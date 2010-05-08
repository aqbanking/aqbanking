INCLUDES = \
  -I$(aqbanking_symlinkdir) \
  -I$(srcdir)/../widgets -I$(srcdir)/.. \
  -I../widgets -I.. \
  $(GWENGUI_QT4_CFLAGS) \
  $(gwenhywfar_includes) $(qt4_includes)

DEFS += -DBUILDING_Q4BANKING
AM_CPPFLAGS= @visibility_cflags@

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


iheaderdir=@aqbanking_headerdir_am@/q4banking
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

MOC_FILES=\
 qbprocesswatcher.moc qbprocesswatcher.ui.moc \
 qbmapaccount.moc qbmapaccount.ui.moc \
 qbimporter.moc qbimporter.ui.moc \
 qbpickstartdate.moc qbpickstartdate.ui.moc \
 qbselectbank.moc qbselectbank.ui.moc \
 qbselectfromlist.moc qbselectfromlist.ui.moc \
 qbprintdialog.moc qbprintdialog.ui.moc \
 qbcfgtabpageusergen.moc qbcfgtabpageusergen.ui.moc \
 qbcfgtabpageaccountgen.moc qbcfgtabpageaccountgen.ui.moc \
 qbcfgtabpageaccounts.moc qbcfgtabpageaccounts.ui.moc \
 qbcfgtabpageusers.moc qbcfgtabpageusers.ui.moc \
 qbcfgtabpagebackends.moc qbcfgtabpagebackends.ui.moc \
 qbcfgtabsettings.moc \
 qbselectbackend.moc qbselectbackend.ui.moc


nodist_libdialogs_la_SOURCES=


BUILT_SOURCES = $(MOC_FILES) $(nodist_libdialogs_la_SOURCES) $(nodist_iheader_HEADERS)
CLEANFILES = $(BUILT_SOURCES)

libdialogs_la_SOURCES= \
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
	for f in $(libdialogs_la_SOURCES) $(foreach uifile,$(UI_FILES),$(uifile).h); do \
	  echo $(subdir)/$$f >>$(top_srcdir)/qtsources; \
	done


%.moc: %.h
	$(qt4_moc) -o $@ $<


%.ui.h: %.ui
	$(qt4_uic) $< -o $@

SUFFIXES = .ui .ui.h .moc

# For qt3 compatibility of qt4
DEFS += -DQT3_SUPPORT

built_sources: $(BUILT_SOURCES)
	list='$(SUBDIRS)'; for d in $$list; do \
	  $(MAKE) -C $$d built_sources; \
	done ;
