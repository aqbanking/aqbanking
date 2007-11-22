INCLUDES = \
  -I$(top_builddir)/headers \
  -I$(srcdir)/../widgets -I$(srcdir)/.. \
  -I../widgets -I.. \
  $(gwenhywfar_includes) $(qt3_includes)

DEFS += -DBUILDING_QBANKING
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


nodist_libdialogs_la_SOURCES=\
 qbprocesswatcher.ui.cpp \
 qbmapaccount.ui.cpp \
 qbimporter.ui.cpp \
 qbpickstartdate.ui.cpp \
 qbselectbank.ui.cpp \
 qbselectfromlist.ui.cpp \
 qbprintdialog.ui.cpp \
 qbcfgtabpageusergen.ui.cpp \
 qbcfgtabpageaccountgen.ui.cpp \
 qbcfgtabpageaccounts.ui.cpp \
 qbcfgtabpageusers.ui.cpp \
 qbcfgtabpagebackends.ui.cpp \
 qbselectbackend.ui.cpp


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
	for f in $(libdialogs_la_SOURCES) $(foreach uifile,$(UI_FILES),$(uifile).cpp); do \
	  echo $(subdir)/$$f >>$(top_srcdir)/qtsources; \
	done


%.moc: %.h
	$(qt3_moc) -o $@ $<

%.ui.cpp: %.ui
	$(UIC3) $< -impl $(@:.ui.cpp=.ui.h) -o $@.tmp && \
	echo "#ifdef HAVE_CONFIG_H" >$@ && \
	echo "# include <config.h>" >>$@ && \
	echo "#endif" >>$@ && \
	cat $@.tmp >>$@ && \
	rm $@.tmp && \
	echo "#include \"$<.moc\"" >>$@


%.ui.h: %.ui
	$(UIC3) $< -o $@

SUFFIXES = .ui .ui.h .ui.cpp .moc

# This section is for a qt4 environment
qt4-port:
	for A in $(libdialogs_la_SOURCES) $(iheader_HEADERS); do \
	 $(YES) | $(QT3TO4) $$A; \
	done
# For qt3 compatibility of qt4
DEFS += -DQT3_SUPPORT

built_sources: $(BUILT_SOURCES)
	list='$(SUBDIRS)'; for d in $$list; do \
	  $(MAKE) -C $$d built_sources; \
	done ;
