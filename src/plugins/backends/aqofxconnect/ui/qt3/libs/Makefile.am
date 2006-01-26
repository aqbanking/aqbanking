INCLUDES=-I$(top_builddir)/headers \
  -I dialogs @all_includes@

UI_FILES=\
 cfgtabpageaccountofx.ui \
 cfgtabpageuserofx.ui

EXTRA_DIST= $(UI_FILES)

qbanking_plugindir=@qbanking_plugindir@/cfgmodules

qbanking_plugin_LTLIBRARIES=aqofxconnect.la

aqofxconnect_la_SOURCES=\
 cfgmoduleofx.cpp \
 cfgtabpageaccountofx.cpp \
 cfgtabpageuserofx.cpp

nodist_aqofxconnect_la_SOURCES=\
 cfgtabpageaccountofx.ui.cpp \
 cfgtabpageaccountofx.ui.moc.cpp \
 cfgtabpageuserofx.ui.cpp \
 cfgtabpageuserofx.ui.moc.cpp \
 cfgtabpageuserofx.moc.cpp

noinst_HEADERS=\
 cfgmoduleofx_p.h \
 cfgtabpageaccountofx.ui.h \
 cfgtabpageuserofx.ui.h

aqofxconnect_la_LDFLAGS=@STRIPALL@ -module

aqofxconnect_la_LIBADD=\
  $(aqofxconnect_internal_libs) \
  $(aqbanking_internal_libs) \
  $(qbanking_internal_libs) \
  $(gwenhywfar_libs) $(qt3_libs)


BUILT_SOURCES = $(nodist_aqofxconnect_la_SOURCES)
CLEANFILES = $(nodist_aqofxconnect_la_SOURCES)

sources:
	for f in $(aqofxconnect_la_SOURCES) $(foreach uifile,$(UI_FILES),$(uifile).cpp); do \
	  echo $(subdir)/$$f >>$(top_srcdir)/qtsources; \
	done
	for d in $(SUBDIRS); do \
	  make -C $$d sources; \
	done


%.moc.cpp: %.h
	@qt3_moc@ -o $@ $<

# Build rules for the files that come from the .ui file
%.ui.cpp: %.ui
	$(UIC3) $< -impl $<.h -o $@
%.ui.h: %.ui
	$(UIC3) $< -o $@
SUFFIXES = .ui .ui.h .ui.cpp .moc.cpp

# This section is for a qt4 environment
qt4-port:
	for d in $(SUBDIRS); do \
	  make -C $$d qt4-port; \
	done
# For qt3 compatibility of qt4
DEFS += -DQT3_SUPPORT

built_sources: $(BUILT_SOURCES)
	for d in $(SUBDIRS); do \
	  make -C $$d built_sources; \
	done ;
