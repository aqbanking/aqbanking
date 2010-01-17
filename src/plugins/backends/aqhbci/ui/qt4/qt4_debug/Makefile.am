INCLUDES = \
 -I$(srcdir)/../libs/widgets -I$(srcdir)/../libs/dialogs \
 -I$(srcdir)/../libs/logmanager -I$(srcdir)/../libs \
 -I../libs/widgets -I../libs/dialogs -I../libs/logmanager -I../libs \
  $(gwenhywfar_includes) $(qt4_includes)

DEFS += -DPKGDATADIR=\"$(pkgdatadir)\"

EXTRA_DIST=qt4_debug.xml.in

plugindir = $(aqbanking_plugindir)/debugger/aqhbci
plugin_PROGRAMS = aqhbci-qt4-debug
plugin_DATA = qt4_debug.xml

#noinst_HEADERS=

aqhbci_qt4_debug_SOURCES = qt4_debug.cpp
aqhbci_qt4_debug_LDADD = \
  ../libs/logmanager/liblogmanager.la \
  $(aqbanking_internal_libs) \
  $(q4banking_internal_libs) \
  $(aqhbci_internal_libs) \
  $(gwenhywfar_libs) $(qt3_libs) -lQtCore


CLEANFILES=qt4_debug.xml


sources:
	for f in $(aqhbci_qt4_debug_SOURCES); do \
	  echo $(subdir)/$$f >>$(top_srcdir)/qtsources; \
	done
	for d in $(SUBDIRS); do \
	  $(MAKE) -C $$d sources; \
	done

# For qt3 compatibility of qt4
DEFS += -DQT3_SUPPORT

built_sources: $(BUILT_SOURCES)

