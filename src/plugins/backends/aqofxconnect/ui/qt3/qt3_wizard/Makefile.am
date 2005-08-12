
INCLUDES = \
 -I$(top_builddir)/headers \
 -I../libs/widgets \
 -I../libs/dialogs \
 -I../libs/views \
 -I../libs $(qt3_includes) $(all_includes)

DEFS += -DDATADIR=\"$(pkgdatadir)\"

EXTRA_DIST = qt_wizard.xml.in

plugindir = $(aqbanking_plugindir)/wizards/aqofxconnect
plugin_PROGRAMS = aqofxconnect-qt3-wizard
plugin_DATA=qt_wizard.xml

#noinst_HEADERS=qt3_wizard.h

aqofxconnect_qt3_wizard_SOURCES = qt3_wizard.cpp
aqofxconnect_qt3_wizard_LDADD =  \
  ../libs/libqt3wizard.la \
  -L$(top_builddir)/src/libs/aqbanking $(aqbanking_internal_libs) \
  -L$(top_builddir)/src/frontends/qbanking -lqbanking \
  -L$(top_builddir)/src/plugins/backends/aqofxconnect/plugin -laqofxconnect \
  $(gwenhywfar_libs) $(qt3_libs)


CLEANFILES=qt_wizard.xml


sources:
	for f in $(aqofxconnect_qt3_wizard_SOURCES); do \
	  echo $(subdir)/$$f >>$(top_srcdir)/qtsources; \
	done
	for d in $(SUBDIRS); do \
	  make -C $$d sources; \
	done

# This section is for a qt4 environment
qt4-port:
	for A in $(aqofxconnect_qt3_wizard_SOURCES) $(noinst_HEADERS); do \
	 $(YES) | $(QT3TO4) $$A; \
	done
#	rm $(BUILT_SOURCES)
# For qt3 compatibility of qt4
DEFS += -DQT3_SUPPORT
