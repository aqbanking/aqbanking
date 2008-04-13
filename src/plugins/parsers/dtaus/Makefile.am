INCLUDES = \
  -I$(top_builddir)/headers $(gwenhywfar_includes) \
  -I$(top_srcdir)/src/libs/aqbanking

EXTRA_DIST=README

noinst_LTLIBRARIES=libparser_dtaus.la
libparser_dtaus_la_SOURCES=\
 dtaus.c \
 dtaus-import.c \
 dtaus-export.c
noinst_HEADERS=\
 dtaus.h \
 dtaus_p.h \
 dtaus-import_p.h \
 dtaus-export_p.h


dbioplugindir = $(gwenhywfar_plugins)/dbio
dbioplugin_LTLIBRARIES=dtaus.la
dbioplugin_DATA=dtaus.xml
dtaus_la_SOURCES=dummy.c
dtaus_la_LIBADD = \
  libparser_dtaus.la \
  $(aqbanking_internal_libs) \
  $(gwenhywfar_libs) 

dtaus_la_LDFLAGS = -no-undefined $(STRIPALL) -module -omit-version


# Add the linker flags for windows resource file
dtaus_la_DEPENDENCIES = libparser_dtaus.la $(aqbanking_internal_libs)
# if IS_WINDOWS
if IS_WINDOWS
dtaus_la_DEPENDENCIES += ressource.o
dtaus_la_LDFLAGS += -Wl,ressource.o

ressource.o: ressource.rc
	$(RC) -o $@ $<
endif
# IS_WINDOWS
# End of linker flags for windows resource file


sources:
	for f in $(libparser_dtaus_la_SOURCES); do \
	  echo $(subdir)/$$f >>$(top_srcdir)/i18nsources; \
	done

