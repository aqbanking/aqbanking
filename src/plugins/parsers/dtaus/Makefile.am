INCLUDES = -I$(top_builddir)/headers $(gwenhywfar_includes) $(all_includes)
EXTRA_DIST=README

dbioplugindir = $(gwenhywfar_plugins)/dbio

dbioplugin_LTLIBRARIES=dtaus.la
if IS_WINDOWS
dbioplugin_DATA=dtaus.xml 
else
dbioplugin_DATA=dtaus.xml
endif

noinst_HEADERS=\
 dtaus.h \
 dtaus_p.h \
 dtaus-import_p.h \
 dtaus-export_p.h

dtaus_la_SOURCES=\
 dtaus.c \
 dtaus-import.c \
 dtaus-export.c

dtaus_la_LIBADD = \
  $(aqbanking_internal_libs) \
  $(gwenhywfar_libs)

dtaus_la_LDFLAGS = -no-undefined $(STRIPALL) -module -omit-version


# Add the linker flags for windows resource file
dtaus_la_DEPENDENCIES = \
 $(aqbanking_internal_libs)
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
	for f in $(dtaus_la_SOURCES); do \
	  echo $(subdir)/$$f >>$(top_srcdir)/i18nsources; \
	done

