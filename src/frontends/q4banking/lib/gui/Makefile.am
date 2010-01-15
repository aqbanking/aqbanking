INCLUDES = \
 $(gwenhywfar_includes) $(qt4_includes) -I$(top_builddir)/headers \
 -I../dialogs -I.. -I$(top_srcdir)/src/libs/aqbanking

DEFS += -DBUILDING_Q4BANKING
AM_CPPFLAGS= @visibility_cflags@

UI_FILES= \
 qguiprogresswidget.ui \
 qguisimplebox.ui

EXTRA_DIST=$(UI_FILES)

noinst_LTLIBRARIES=libqgui.la

nodist_iheader_HEADERS=\
 qguiprogresswidget.ui.h \
 qguisimplebox.ui.h

iheaderdir=${includedir}/q4banking
iheader_HEADERS=\
 cppgui.h \
 qgui.h \
 qguisimplebox.h \
 qguiinputbox.h \
 qguiprogress.h \
 qguiprogresswidget.h

noinst_HEADERS=\
 cppgui_p.h

MOC_FILES=\
 qguiprogresswidget.moc qguiprogresswidget.ui.moc \
 qguisimplebox.moc qguisimplebox.ui.moc \
 qguiinputbox.moc

nodist_libqgui_la_SOURCES=

libqgui_la_SOURCES=\
 cppgui.cpp \
 qgui.cpp \
 qguiinputbox.cpp \
 qguiprogress.cpp  \
 qguiprogresswidget.cpp  \
 qguisimplebox.cpp


BUILT_SOURCES = $(MOC_FILES) $(nodist_libqgui_la_SOURCES) $(nodist_iheader_HEADERS)
CLEANFILES = $(BUILT_SOURCES)

sources:
	for f in $(libqgui_la_SOURCES) $(foreach uifile,$(UI_FILES),$(uifile).cpp); do \
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

