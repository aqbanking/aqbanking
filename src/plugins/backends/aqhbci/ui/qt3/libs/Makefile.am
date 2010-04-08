INCLUDES = \
  -I$(top_builddir)/headers \
  -I$(srcdir)/dialogs -I$(srcdir)/logmanager \
  -Idialogs -Ilogmanager \
  $(GWENGUI_QT3_CFLAGS) \
  $(all_includes)

SUBDIRS=dialogs logmanager 
#widgets

qbanking_plugindir=@qbanking_plugindir@/cfgmodules
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
  $(qbanking_internal_libs) \
  $(gwenhywfar_libs) $(qt3_libs)


BUILT_SOURCES = $(nodist_aqhbci_la_SOURCES)
CLEANFILES = $(nodist_aqhbci_la_SOURCES)

sources:
	for d in $(SUBDIRS); do \
	  $(MAKE) -C $$d sources; \
	done


%.moc.cpp: %.h
	@qt3_moc@ -o $@ $<

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
