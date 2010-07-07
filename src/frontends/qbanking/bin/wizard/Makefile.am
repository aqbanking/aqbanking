INCLUDES = \
  -I$(aqbanking_symlinkdir) \
  -I$(srcdir)/../../gui \
  -I$(srcdir)/../../lib/widgets \
  -I$(srcdir)/../../lib/dialogs \
  -I$(srcdir)/../../lib/views \
  -I$(srcdir)/../../lib/gui \
  -I$(srcdir)/../../lib \
  -I../../lib/widgets -I../../lib/dialogs -I../../lib/gui -I../../lib/views -I../../lib \
  $(GWENGUI_QT3_CFLAGS) \
  $(qt3_includes) $(all_includes)

EXTRA_DIST = qt3_wizard.xml.in

plugindir = $(aqbanking_plugindir)/wizards
plugin_PROGRAMS = qt3-wizard
plugin_DATA=qt3_wizard.xml

#noinst_HEADERS=qt3_wizard.h

qt3_wizard_SOURCES = qt3_wizard.cpp
qt3_wizard_LDADD = \
  $(qbanking_internal_libs) \
  $(aqbanking_internal_libs) \
  $(gwenhywfar_libs) $(qt3_libs)


CLEANFILES=qt3_wizard.xml


sources:
	for f in $(qt3_wizard_SOURCES); do \
	  echo $(subdir)/$$f >>$(top_srcdir)/qtsources; \
	done
	for d in $(SUBDIRS); do \
	  $(MAKE) -C $$d sources; \
	done

DEFS += -DQT3_SUPPORT

built_sources:

