INCLUDES = \
 $(gwenhywfar_includes) $(qt3_includes) -I$(top_builddir)/headers \
 -I../dialogs -I.. -I$(top_srcdir)/src/libs/aqbanking

DEFS += -DBUILDING_QBANKING
AM_CPPFLAGS= @visibility_cflags@

UI_FILES= \
 qguiprogresswidget.ui \
 qguisimplebox.ui

EXTRA_DIST=$(UI_FILES)

noinst_LTLIBRARIES=libqgui.la

nodist_iheader_HEADERS=\
 qguiprogresswidget.ui.h \
 qguisimplebox.ui.h

iheaderdir=${includedir}/qbanking
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

nodist_libqgui_la_SOURCES=\
 qguiprogresswidget.ui.cpp \
 qguisimplebox.ui.cpp

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
	for A in $(libqgui_la_SOURCES) $(iheader_HEADERS); do \
	 $(YES) | $(QT3TO4) $$A; \
	done
# For qt3 compatibility of qt4
DEFS += -DQT3_SUPPORT


built_sources: $(BUILT_SOURCES)

