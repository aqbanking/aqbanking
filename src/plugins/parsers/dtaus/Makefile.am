INCLUDES = \
  -I$(aqbanking_symlinkdir) $(gwenhywfar_includes) \
  -I$(top_srcdir)/src/libs/aqbanking

EXTRA_DIST=README

noinst_HEADERS=\
 dtaus.h \
 dtaus_p.h \
 dtaus-import_p.h \
 dtaus-export_p.h


dbioplugindir = $(aqbanking_plugindir)/dbio
dbioplugin_LTLIBRARIES=dtaus.la
dbioplugin_DATA=dtaus.xml
dtaus_la_SOURCES=\
 dtaus.c \
 dtaus-import.c \
 dtaus-export.c

dtaus_la_LIBADD = $(aqbanking_internal_libs) $(gwenhywfar_libs) 
dtaus_la_LDFLAGS = -no-undefined $(STRIPALL) -module -avoid-version


sources:
	for f in $(dtaus_la_SOURCES); do \
	  echo $(subdir)/$$f >>$(top_srcdir)/i18nsources; \
	done
	for f in $(dbioplugin_DATA); do \
	  echo $(subdir)/$$f >>$(top_srcdir)/pdsources; \
	done

