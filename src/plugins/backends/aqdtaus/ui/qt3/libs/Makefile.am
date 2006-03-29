INCLUDES = \
  -I$(top_builddir)/headers \
  -I$(srcdir)/dialogs -I$(srcdir)/widgets -I$(srcdir)/logmanager \
  $(all_includes)

SUBDIRS=dialogs
#widgets

qbanking_plugindir=@qbanking_plugindir@/cfgmodules
qbanking_plugin_LTLIBRARIES=aqdtaus.la

aqdtaus_la_SOURCES=cfgmoduledtaus.cpp

noinst_HEADERS=\
 cfgmoduledtaus_p.h


aqdtaus_la_LDFLAGS=@STRIPALL@ -module -no-undefined

aqdtaus_la_LIBADD=\
  dialogs/libdialogs.la \
  $(aqdtaus_internal_libs) \
  $(aqbanking_internal_libs) \
  $(qbanking_internal_libs) \
  $(gwenhywfar_libs) $(qt3_libs)



BUILT_SOURCES = $(nodist_aqdtaus_la_SOURCES)
CLEANFILES = $(nodist_aqdtaus_la_SOURCES)

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
