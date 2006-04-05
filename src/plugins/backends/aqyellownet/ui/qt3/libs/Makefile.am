INCLUDES=-I$(top_builddir)/headers \
  -I dialogs @all_includes@

UI_FILES=\
 cfgtabpageaccountyn.ui \
 cfgtabpageuseryn.ui

EXTRA_DIST= $(UI_FILES)

qbanking_plugindir=@qbanking_plugindir@/cfgmodules

qbanking_plugin_LTLIBRARIES=aqyellownet.la

noinst_HEADERS=\
 cfgmoduleyn_p.h \
 cfgtabpageaccountyn.h \
 cfgtabpageuseryn.h

nodist_noinst_HEADERS=\
 cfgtabpageaccountyn.ui.h \
 cfgtabpageuseryn.ui.h

nodist_aqyellownet_la_SOURCES=\
 cfgtabpageaccountyn.ui.cpp \
 cfgtabpageaccountyn.ui.moc.cpp \
 cfgtabpageuseryn.ui.cpp \
 cfgtabpageuseryn.ui.moc.cpp

aqyellownet_la_SOURCES= \
 cfgmoduleyn.cpp \
 cfgtabpageaccountyn.cpp \
 cfgtabpageuseryn.cpp


aqyellownet_la_LDFLAGS=@STRIPALL@ -module

aqyellownet_la_LIBADD=\
  $(aqyellownet_internal_libs) \
  $(aqbanking_internal_libs) \
  $(qbanking_internal_libs) \
  $(gwenhywfar_libs) $(qt3_libs)


BUILT_SOURCES = $(nodist_aqyellownet_la_SOURCES)
CLEANFILES = $(nodist_aqyellownet_la_SOURCES)  $(nodist_noinst_HEADERS)

sources:
	for f in $(aqyellownet_la_SOURCES) $(foreach uifile,$(UI_FILES),$(uifile).cpp); do \
	  echo $(subdir)/$$f >>$(top_srcdir)/qtsources; \
	done
	for d in $(SUBDIRS); do \
	  $(MAKE) -C $$d sources; \
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
	  $(MAKE) -C $$d qt4-port; \
	done
# For qt3 compatibility of qt4
DEFS += -DQT3_SUPPORT

built_sources: $(BUILT_SOURCES)
	for d in $(SUBDIRS); do \
	  $(MAKE) -C $$d built_sources; \
	done ;
