AM_CPPFLAGS = \
  $(gwenhywfar_includes) \
  -I$(top_builddir)/src/libs \
  -I$(top_srcdir)/src/libs

AM_CFLAGS=-DBUILDING_AQBANKING @visibility_cflags@

EXTRA_DIST=README

noinst_HEADERS=\
  swift.h \
  swift_l.h \
  swift_p.h \
  swift940_l.h \
  swift940_p.h \
  swift940_60.h \
  swift940_61.h \
  swift940_86.h \
  swift535_l.h \
  swift535_p.h

dbioplugindir = $(aqbanking_plugindir)/dbio
dbioplugin_DATA=swift.xml
dbioplugin_LTLIBRARIES=swift.la
swift_la_SOURCES=\
 swift.c \
 swift940.c \
 swift940_60.c \
 swift940_61.c \
 swift940_86.c \
 swift535.c

swift_la_LIBADD = $(gwenhywfar_libs) 
swift_la_LDFLAGS = -no-undefined $(STRIPALL) -module -avoid-version


sources:
	for f in $(swift_la_SOURCES); do \
	  echo $(subdir)/$$f >>$(top_srcdir)/i18nsources; \
	done
	for f in $(dbioplugin_DATA); do \
	  echo $(subdir)/$$f >>$(top_srcdir)/pdsources; \
	done


cppcheck:
	for f in $(swift_la_SOURCES); do \
	  cppcheck --force $$f ; \
	done
	for d in $(SUBDIRS); do \
	  $(MAKE) -C $$d cppcheck; \
	done


