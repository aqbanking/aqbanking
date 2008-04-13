INCLUDES = \
  -I$(top_builddir)/headers $(gwenhywfar_includes) \
  -I$(top_srcdir)/src/libs/aqbanking

EXTRA_DIST=README

noinst_HEADERS=\
  swift.h \
  swift_l.h \
  swift_p.h \
  swift940_l.h \
  swift940_p.h
noinst_LTLIBRARIES=libparser_swift.la
libparser_swift_la_SOURCES=\
 swift.c \
 swift940.c


dbioplugindir = $(gwenhywfar_plugins)/dbio
dbioplugin_DATA=swift.xml
dbioplugin_LTLIBRARIES=swift.la
swift_la_SOURCES=dummy.c

swift_la_LIBADD = \
  libparser_swift.la \
  $(aqbanking_internal_libs) \
  $(gwenhywfar_libs) 

swift_la_LDFLAGS = -no-undefined $(STRIPALL) -module -omit-version


# Add the linker flags for windows resource file
swift_la_DEPENDENCIES = libparser_swift.la $(aqbanking_internal_libs)
# if IS_WINDOWS
if IS_WINDOWS
swift_la_DEPENDENCIES += ressource.o
swift_la_LDFLAGS += -Wl,ressource.o
ressource.o: ressource.rc
	$(RC) -o $@ $<
endif
# IS_WINDOWS
# End of linker flags for windows resource file


sources:
	for f in $(libparser_swift_la_SOURCES); do \
	  echo $(subdir)/$$f >>$(top_srcdir)/i18nsources; \
	done
