INCLUDES=-I$(top_builddir)/headers \
  $(GWENGUI_QT4_CFLAGS) \
  -I dialogs $(gwenhywfar_includes) $(qt4_includes)

UI_FILES=\
 cfgtabpageaccountofx.ui \
 cfgtabpageuserofx.ui

EXTRA_DIST= $(UI_FILES)

qbanking_plugindir=@q4banking_plugindir@/cfgmodules

qbanking_plugin_LTLIBRARIES=aqofxconnect.la

aqofxconnect_la_SOURCES=\
 cfgmoduleofx.cpp \
 cfgtabpageaccountofx.cpp \
 cfgtabpageuserofx.cpp

nodist_aqofxconnect_la_SOURCES=

noinst_HEADERS=\
 cfgmoduleofx_p.h \
 cfgtabpageaccountofx.h \
 cfgtabpageuserofx.h

nodist_noinst_HEADERS=\
 cfgtabpageaccountofx.ui.h \
 cfgtabpageuserofx.ui.h

aqofxconnect_la_LDFLAGS=@STRIPALL@ -module

aqofxconnect_la_LIBADD=\
  $(aqofxconnect_internal_libs) \
  $(aqbanking_internal_libs) \
  $(q4banking_internal_libs) \
  $(gwenhywfar_libs) $(qt3_libs)

MOC_FILES=\
 cfgtabpageaccountofx.moc \
 cfgtabpageuserofx.moc


BUILT_SOURCES = $(nodist_aqofxconnect_la_SOURCES) $(MOC_FILES) $(nodist_noinst_HEADERS)
CLEANFILES = $(BUILT_SOURCES);

sources:
	for f in $(aqofxconnect_la_SOURCES) $(foreach uifile,$(UI_FILES),$(uifile).cpp); do \
	  echo $(subdir)/$$f >>$(top_srcdir)/qtsources; \
	done

%.moc: %.h
	$(qt4_moc) -o $@ $<

# Build rules for the files that come from the .ui file
%.ui.h: %.ui
	$(qt4_uic) $< -o $@

SUFFIXES = .ui .ui.h .moc

# For qt3 compatibility of qt4
DEFS += -DQT3_SUPPORT

built_sources: $(BUILT_SOURCES)
