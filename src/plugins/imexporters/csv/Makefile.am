SUBDIRS=profiles

INCLUDES = -I$(top_srcdir)/src/libs/aqbanking -I$(top_builddir)/headers $(all_includes)

abs_top_builddir=$(shell (cd $(top_builddir) && pwd) )

EXTRA_DIST=README

imexporterplugindir = $(aqbanking_plugindir)/imexporters

imexporterplugin_LTLIBRARIES=csv.la
if IS_WINDOWS
imexporterplugin_DATA=csv.xml csv.dll
else
imexporterplugin_DATA=csv.xml
endif

noinst_HEADERS=\
 csv_p.h

csv_la_SOURCES=\
 csv.c

csv_la_LIBADD=$(aqbanking_internal_libs) $(gwenhywfar_libs) 
csv_la_LDFLAGS = -no-undefined @STRIPALL@ -module -omit-version

# Add the linker flags for windows resource file
csv_la_DEPENDENCIES = \
 $(aqbanking_internal_libs)
# if IS_WINDOWS
if IS_WINDOWS
csv_la_DEPENDENCIES += ressource.o
csv_la_LDFLAGS += -Wl,ressource.o
ressource.o: ressource.rc
	$(RC) -o $@ $<
endif
# IS_WINDOWS
# End of linker flags for windows resource file

sources:
	for f in $(csv_la_SOURCES); do \
	  echo $(subdir)/$$f >>$(top_srcdir)/i18nsources; \
	done
