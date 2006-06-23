SUBDIRS=profiles

INCLUDES = -I$(top_srcdir)/src/libs/aqbanking -I$(top_builddir)/headers $(all_includes)

EXTRA_DIST=README

imexporterplugindir = $(aqbanking_plugindir)/imexporters

imexporterplugin_LTLIBRARIES=xmldb.la
if IS_WINDOWS
imexporterplugin_DATA=xmldb.xml 
else
imexporterplugin_DATA=xmldb.xml
endif

noinst_HEADERS=\
 xmldb_p.h

xmldb_la_SOURCES=\
 xmldb.c

xmldb_la_LIBADD=$(aqbanking_internal_libs) $(gwenhywfar_libs) 
xmldb_la_LDFLAGS = -no-undefined @STRIPALL@ -module -omit-version

# Add the linker flags for windows resource file
xmldb_la_DEPENDENCIES = \
 $(aqbanking_internal_libs)
# if IS_WINDOWS
if IS_WINDOWS
xmldb_la_DEPENDENCIES += ressource.o
xmldb_la_LDFLAGS += -Wl,ressource.o
ressource.o: ressource.rc
	$(RC) -o $@ $<
endif
# IS_WINDOWS
# End of linker flags for windows resource file


sources:
	for f in $(xmldb_la_SOURCES); do \
	  echo $(subdir)/$$f >>$(top_srcdir)/i18nsources; \
	done
