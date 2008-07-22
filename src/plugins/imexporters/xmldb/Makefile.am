SUBDIRS=profiles

INCLUDES = -I$(top_srcdir)/src/libs/aqbanking -I$(top_builddir)/headers $(all_includes)

EXTRA_DIST=README

noinst_HEADERS=xmldb_p.h
noinst_LTLIBRARIES=libio_xmldb.la
libio_xmldb_la_SOURCES=xmldb.c

imexporterplugindir = $(aqbanking_plugindir)/imexporters
imexporterplugin_LTLIBRARIES=xmldb.la
imexporterplugin_DATA=xmldb.xml

xmldb_la_SOURCES=dummy.c
xmldb_la_LIBADD=libio_xmldb.la $(aqbanking_internal_libs) $(gwenhywfar_libs) 
xmldb_la_LDFLAGS = -no-undefined @STRIPALL@ -module -avoid-version

# Add the linker flags for windows resource file
xmldb_la_DEPENDENCIES = libio_xmldb.la $(aqbanking_internal_libs)
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
	for f in $(libio_xmldb_la_SOURCES); do \
	  echo $(subdir)/$$f >>$(top_srcdir)/i18nsources; \
	done
