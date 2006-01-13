INCLUDES = \
  -I$(top_builddir)/headers \
  -I$(srcdir)/../../lib/widgets \
  -I$(srcdir)/../../lib/dialogs \
  -I$(srcdir)/../../lib/views \
  -I$(srcdir)/../../lib \
  -I../../lib/widgets -I../../lib/dialogs -I../../lib/views -I../../lib \
  $(qt3_includes) $(all_includes)

DEFS += -DPKGDATADIR=\"$(pkgdatadir)\"

EXTRA_DIST = qt3_wizard.xml.in

plugindir = $(aqbanking_plugindir)/wizards
plugin_PROGRAMS = qt3-wizard
plugin_DATA=qt3_wizard.xml

#noinst_HEADERS=qt3_wizard.h

qt3_wizard_SOURCES = qt3_wizard.cpp
qt3_wizard_LDADD = \
  $(aqbanking_internal_libs) \
  $(qbanking_internal_libs) \
  $(gwenhywfar_libs) $(qt3_libs)


CLEANFILES=qt3_wizard.xml


sources:
	for f in $(aqdtaus_qt3_wizard_SOURCES); do \
	  echo $(subdir)/$$f >>$(top_srcdir)/qtsources; \
	done
	for d in $(SUBDIRS); do \
	  make -C $$d sources; \
	done

# This section is for a qt4 environment
qt4-port:
	for A in $(aqdtaus_qt3_wizard_SOURCES) $(noinst_HEADERS); do \
	 $(YES) | $(QT3TO4) $$A; \
	done
#	rm $(BUILT_SOURCES)
# For qt3 compatibility of qt4
DEFS += -DQT3_SUPPORT

built_sources:

