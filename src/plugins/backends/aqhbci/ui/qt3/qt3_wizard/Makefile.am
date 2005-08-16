INCLUDES = \
  -I$(top_builddir)/headers \
  -I$(srcdir)/../libs/widgets -I$(srcdir)/../libs/dialogs \
  -I$(srcdir)/../libs \
  -I../libs/widgets -I../libs/dialogs -I../libs \
  $(qt3_includes) $(all_includes)

DEFS += -DDATADIR=\"$(pkgdatadir)\"

EXTRA_DIST = kde_wizard.xml.in aqhbci_qt_wizard.vcproj

plugindir = $(aqbanking_plugindir)/wizards/aqhbci
plugin_PROGRAMS = aqhbci-qt3-wizard
plugin_DATA=kde_wizard.xml

#noinst_HEADERS=qt3_wizard.h

aqhbci_qt3_wizard_SOURCES = qt3_wizard.cpp
aqhbci_qt3_wizard_LDADD = \
  $(aqhbci_internal_libs) \
  $(aqbanking_internal_libs) \
  $(qbanking_internal_libs) \
  ../libs/libqt3wizard.la \
  $(gwenhywfar_libs) $(qt3_libs)


CLEANFILES=kde_wizard.xml


sources:
	for f in $(aqhbci_qt3_wizard_SOURCES); do \
	  echo $(subdir)/$$f >>$(top_srcdir)/qtsources; \
	done
	for d in $(SUBDIRS); do \
	  make -C $$d sources; \
	done

# This section is for a qt4 environment
qt4-port:
	for A in $(aqhbci_qt3_wizard_SOURCES) $(noinst_HEADERS); do \
	 $(YES) | $(QT3TO4) $$A; \
	done
#	rm $(BUILT_SOURCES)
# For qt3 compatibility of qt4
DEFS += -DQT3_SUPPORT

built_sources:

