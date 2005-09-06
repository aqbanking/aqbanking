INCLUDES = \
 -I$(srcdir)/../libs/widgets -I$(srcdir)/../libs/dialogs \
 -I$(srcdir)/../libs/logmanager -I$(srcdir)/../libs \
 -I../libs/widgets -I../libs/dialogs -I../libs/logmanager -I../libs \
 $(qt3_includes) $(all_includes)

DEFS += -DPKGDATADIR=\"$(pkgdatadir)\"

EXTRA_DIST=qt_debug.xml.in

plugindir = $(aqbanking_plugindir)/debugger/aqhbci
plugin_PROGRAMS = aqhbci-qt3-debug
plugin_DATA = qt_debug.xml

#noinst_HEADERS=

aqhbci_qt3_debug_SOURCES = qt3_debug.cpp
aqhbci_qt3_debug_LDADD = \
  ../libs/libqt3wizard.la \
  $(aqbanking_internal_libs) \
  $(qbanking_internal_libs) \
  $(aqhbci_internal_libs) \
  $(gwenhywfar_libs) $(qt3_libs)


CLEANFILES=qt_debug.xml


sources:
	for f in $(aqhbci_qt3_debug_SOURCES); do \
	  echo $(subdir)/$$f >>$(top_srcdir)/qtsources; \
	done
	for d in $(SUBDIRS); do \
	  make -C $$d sources; \
	done

# This section is for a qt4 environment
qt4-port:
	for A in $(aqhbci_qt3_debug_SOURCES) $(noinst_HEADERS); do \
	 $(YES) | $(QT3TO4) $$A; \
	done
#	rm $(BUILT_SOURCES)
# For qt3 compatibility of qt4
DEFS += -DQT3_SUPPORT

built_sources: $(BUILT_SOURCES)

