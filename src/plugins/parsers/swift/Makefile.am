INCLUDES = \
  -I$(aqbanking_symlinkdir) $(gwenhywfar_includes) \
  -I$(top_srcdir)/src/libs/aqbanking

EXTRA_DIST=README

noinst_HEADERS=\
  swift.h \
  swift_l.h \
  swift_p.h \
  swift940_l.h \
  swift940_p.h

dbioplugindir = $(aqbanking_plugindir)/dbio
dbioplugin_DATA=swift.xml
dbioplugin_LTLIBRARIES=swift.la
swift_la_SOURCES=\
 swift.c \
 swift940.c

swift_la_LIBADD = $(aqbanking_internal_libs) $(gwenhywfar_libs) 
swift_la_LDFLAGS = -no-undefined $(STRIPALL) -module -avoid-version


sources:
	for f in $(swift_la_SOURCES); do \
	  echo $(subdir)/$$f >>$(top_srcdir)/i18nsources; \
	done
	for f in $(dbioplugin_DATA); do \
	  echo $(subdir)/$$f >>$(top_srcdir)/pdsources; \
	done
