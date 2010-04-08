INCLUDES = \
  -I$(top_builddir)/headers \
  -I$(srcdir)/.. -I$(srcdir)/../dialogs \
  -I.. -I../dialogs \
  $(GWENGUI_QT4_CFLAGS) \
  $(gwenhywfar_includes) $(qt4_includes)

DEFS += -DBUILDING_Q4BANKING
AM_CPPFLAGS= @visibility_cflags@

UI_FILES= \
 qbcfgtab.ui

EXTRA_DIST=$(UI_FILES)

noinst_LTLIBRARIES=libwidgets.la

nodist_iheader_HEADERS=\
 qbcfgtab.ui.h

iheaderdir=${includedir}/q4banking
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

nodist_libwidgets_la_SOURCES=

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
	$(qt4_moc) -o $@ $<

%.ui.h: %.ui
	$(qt4_uic) $< -o $@

SUFFIXES = .ui .ui.h .moc


# For qt4 compatibility of qt4
DEFS += -DQT3_SUPPORT


built_sources: $(BUILT_SOURCES)

