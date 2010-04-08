INCLUDES = \
  -I$(top_builddir)/headers \
  -I$(srcdir)/dialogs -I$(srcdir)/logmanager \
  -Idialogs -Ilogmanager \
  $(GWENGUI_QT4_CFLAGS) \
  $(gwenhywfar_includes) $(qt4_includes)

SUBDIRS=dialogs logmanager 
#widgets

qbanking_plugindir=@q4banking_plugindir@/cfgmodules
qbanking_plugin_LTLIBRARIES=aqhbci.la

aqhbci_la_SOURCES=cfgmodulehbci.cpp

noinst_HEADERS=\
 cfgmodulehbci_p.h


aqhbci_la_LDFLAGS=@STRIPALL@ -module -no-undefined

aqhbci_la_LIBADD=\
  dialogs/libdialogs.la \
  logmanager/liblogmanager.la \
  $(aqhbci_internal_libs) \
  $(aqbanking_internal_libs) \
  $(q4banking_internal_libs) \
  $(gwenhywfar_libs) $(qt4_libs) -lQt3Support


BUILT_SOURCES = $(nodist_aqhbci_la_SOURCES)
CLEANFILES = $(nodist_aqhbci_la_SOURCES)

sources:
	for d in $(SUBDIRS); do \
	  $(MAKE) -C $$d sources; \
	done


%.moc.cpp: %.h
	@qt3_moc@ -o $@ $<

%.ui.h: %.ui
	$(qt4_uic) $< -o $@

# For qt3 compatibility of qt4
DEFS += -DQT3_SUPPORT

built_sources: $(BUILT_SOURCES)
	for d in $(SUBDIRS); do \
	  $(MAKE) -C $$d built_sources; \
	done ;
