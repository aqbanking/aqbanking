INCLUDES = \
  -I$(top_builddir)/headers \
  -I$(srcdir)/dialogs -I$(srcdir)/widgets -I$(srcdir)/logmanager \
  $(all_includes)

SUBDIRS=dialogs logmanager widgets

noinst_LTLIBRARIES=libqt3wizard.la

libqt3wizard_la_SOURCES=dummy.cpp

libqt3wizard_la_LDFLAGS=@STRIPALL@

libqt3wizard_la_LIBADD=\
  dialogs/libdialogs.la \
  widgets/libwidgets.la \
  logmanager/liblogmanager.la


BUILT_SOURCES = $(nodist_libqt3wizard_la_SOURCES)
CLEANFILES = $(nodist_libqt3wizard_la_SOURCES)

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
