INCLUDES = \
  -I$(top_builddir)/headers \
  -I$(srcdir)/.. -I$(srcdir)/../dialogs \
  -I.. -I../dialogs \
  $(all_includes)

DEFS += -DBUILDING_QBANKING
AM_CPPFLAGS= @visibility_cflags@

UI_FILES= \
 qbprogress.ui \
 qbsimplebox.ui \
 qbcfgtab.ui

EXTRA_DIST=$(UI_FILES)

noinst_LTLIBRARIES=libwidgets.la

nodist_iheader_HEADERS=\
 qbprogress.ui.h \
 qbsimplebox.ui.h \
 qbcfgtab.ui.h

iheaderdir=${includedir}/qbanking
iheader_HEADERS=\
 qbsimplebox.h \
 qbaccountlist.h \
 qbinputbox.h \
 qbjoblist.h \
 qbplugindescrlist.h \
 qbprogress.h \
 qbcfgtab.h qbcfgtabpage.h \
 qbcfgtabpageuser.h \
 qbcfgtabpageaccount.h \
 qbuserlist.h

MOC_FILES=\
 qbprogress.moc qbprogress.ui.moc \
 qbsimplebox.moc qbsimplebox.ui.moc \
 qbcfgtab.moc qbcfgtab.ui.moc \
 qbinputbox.moc

nodist_libwidgets_la_SOURCES=\
 qbprogress.ui.cpp \
 qbsimplebox.ui.cpp \
 qbcfgtab.ui.cpp

libwidgets_la_SOURCES=\
 qbaccountlist.cpp \
 qbjoblist.cpp \
 qbinputbox.cpp \
 qbplugindescrlist.cpp \
 qbprogress.cpp  \
 qbsimplebox.cpp \
 qbcfgtab.cpp \
 qbcfgtabpage.cpp \
 qbcfgtabpageuser.cpp \
 qbcfgtabpageaccount.cpp \
 qbuserlist.cpp



BUILT_SOURCES = $(MOC_FILES) $(nodist_libwidgets_la_SOURCES) $(nodist_iheader_HEADERS)
CLEANFILES = $(BUILT_SOURCES)

sources:
	for f in $(libwidgets_la_SOURCES) $(foreach uifile,$(UI_FILES),$(uifile).cpp); do \
	  echo $(subdir)/$$f >>$(top_srcdir)/qtsources; \
	done

%.moc: %.h
	$(qt3_moc) -o $@ $<

%.ui.cpp: %.ui
	$(UIC3) $< -impl $<.h -o $<.tmp && \
	echo "#ifdef HAVE_CONFIG_H" >$@ && \
	echo "# include <config.h>" >>$@ && \
	echo "#endif" >>$@ && \
	cat $<.tmp >>$@ && \
	rm $<.tmp && \
	echo "#include \"$<.moc\"" >>$@


%.ui.h: %.ui
	$(UIC3) $< -o $@

SUFFIXES = .ui .ui.h .ui.cpp .moc


# This section is for a qt4 environment
qt4-port:
	for A in $(libwidgets_la_SOURCES) $(iheader_HEADERS); do \
	 $(YES) | $(QT3TO4) $$A; \
	done
# For qt3 compatibility of qt4
DEFS += -DQT3_SUPPORT


built_sources: $(BUILT_SOURCES)

