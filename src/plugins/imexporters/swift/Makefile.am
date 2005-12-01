SUBDIRS=profiles

INCLUDES = -I$(top_srcdir)/src/libs/aqbanking -I$(top_builddir)/headers $(all_includes)

EXTRA_DIST=README

imexporterplugindir = $(aqbanking_plugindir)/imexporters

imexporterplugin_LTLIBRARIES=swift.la
if IS_WINDOWS
imexporterplugin_DATA=swift.xml 
else
imexporterplugin_DATA=swift.xml
endif

noinst_HEADERS=\
 swift_p.h

swift_la_SOURCES=\
 swift.c

swift_la_LIBADD=$(aqbanking_internal_libs) $(gwenhywfar_libs) 
swift_la_LDFLAGS = -no-undefined @STRIPALL@ -module -omit-version

# Add the linker flags for windows resource file
swift_la_DEPENDENCIES = \
 $(aqbanking_internal_libs)
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
	for f in $(swift_la_SOURCES); do \
	  echo $(subdir)/$$f >>$(top_srcdir)/i18nsources; \
	done
