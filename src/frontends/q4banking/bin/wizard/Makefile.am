INCLUDES = \
  -I$(top_builddir)/headers \
  -I$(srcdir)/../../gui \
  -I$(srcdir)/../../lib/widgets \
  -I$(srcdir)/../../lib/dialogs \
  -I$(srcdir)/../../lib/views \
  -I$(srcdir)/../../lib/gui \
  -I$(srcdir)/../../lib \
  -I../../lib/widgets -I../../lib/dialogs -I../../lib/gui -I../../lib/views -I../../lib \
  $(qt4_includes) $(all_includes)

EXTRA_DIST = qt4_wizard.xml.in

plugindir = $(aqbanking_plugindir)/wizards
plugin_PROGRAMS = qt4-wizard
plugin_DATA=qt4_wizard.xml

#noinst_HEADERS=qt4_wizard.h

qt4_wizard_SOURCES = qt4_wizard.cpp
qt4_wizard_LDADD = \
  $(q4banking_internal_libs) \
  $(aqbanking_internal_libs) \
  $(gwenhywfar_libs) $(qt4_libs)


CLEANFILES=qt4_wizard.xml


sources:
	for f in $(qt4_wizard_SOURCES); do \
	  echo $(subdir)/$$f >>$(top_srcdir)/qtsources; \
	done
	for d in $(SUBDIRS); do \
	  $(MAKE) -C $$d sources; \
	done

# For qt3 compatibility of qt4
DEFS += -DQT3_SUPPORT

built_sources:

