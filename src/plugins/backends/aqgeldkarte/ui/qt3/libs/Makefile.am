INCLUDES = \
  -I$(top_builddir)/headers \
  -I$(srcdir)/dialogs \
  -Idialogs \
  $(all_includes)

SUBDIRS=dialogs

qbanking_plugindir=@qbanking_plugindir@/cfgmodules
qbanking_plugin_LTLIBRARIES=aqgeldkarte.la

aqgeldkarte_la_SOURCES=cfgmodulegeldkarte.cpp

noinst_HEADERS=\
 cfgmodulegeldkarte_p.h


aqgeldkarte_la_LDFLAGS=@STRIPALL@ -module

aqgeldkarte_la_LIBADD=\
  dialogs/libdialogs.la \
  $(aqgeldkarte_internal_libs) \
  $(aqbanking_internal_libs) \
  $(qbanking_internal_libs) \
  $(gwenhywfar_libs) $(qt3_libs)


BUILT_SOURCES = $(nodist_aqgeldkarte_la_SOURCES)
CLEANFILES = $(nodist_aqgeldkarte_la_SOURCES)

sources:
	for d in $(SUBDIRS); do \
	  make -C $$d sources; \
	done


%.moc.cpp: %.h
	@qt3_moc@ -o $@ $<

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
