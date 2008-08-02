INCLUDES = \
  -I$(top_builddir)/headers \
  -I$(srcdir)/.. -I$(srcdir)/../dialogs \
  -I.. -I../dialogs \
  $(gwenhywfar_includes) $(qt3_includes)

DEFS += -DBUILDING_QBANKING
AM_CPPFLAGS= @visibility_cflags@

UI_FILES= \
 qbcfgtab.ui

EXTRA_DIST=$(UI_FILES)

noinst_LTLIBRARIES=libwidgets.la

nodist_iheader_HEADERS=\
 qbcfgtab.ui.h

iheaderdir=${includedir}/qbanking
iheader_HEADERS=\
 qbaccountlist.h \
 qbjoblist.h \
 qbplugindescrlist.h \
 qbcfgtab.h qbcfgtabpage.h \
 qbcfgtabpageuser.h \
 qbcfgtabpageaccount.h \
 qbuserlist.h

MOC_FILES=\
 qbcfgtab.moc qbcfgtab.ui.moc

nodist_libwidgets_la_SOURCES=\
 qbcfgtab.ui.cpp

libwidgets_la_SOURCES=\
 qbaccountlist.cpp \
 qbjoblist.cpp \
 qbplugindescrlist.cpp \
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
	$(UIC3) $< -impl $(@:.ui.cpp=.ui.h) -o $@.tmp && \
	echo "#ifdef HAVE_CONFIG_H" >$@ && \
	echo "# include <config.h>" >>$@ && \
	echo "#endif" >>$@ && \
	cat $@.tmp >>$@ && \
	rm $@.tmp && \
	echo "#include \"`basename $<`.moc\"" >>$@


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

