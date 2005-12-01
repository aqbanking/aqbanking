SUBDIRS=profiles

INCLUDES = -I$(top_srcdir)/src/libs/aqbanking -I$(top_builddir)/headers $(all_includes)
#EXTRA_DIST=README

imexporterplugindir = $(aqbanking_plugindir)/imexporters

imexporterplugin_LTLIBRARIES=qif.la
if IS_WINDOWS
imexporterplugin_DATA=qif.xml 
else
imexporterplugin_DATA=qif.xml
endif

noinst_HEADERS=\
 qif_p.h

qif_la_SOURCES=\
 qif.c

qif_la_LIBADD=$(aqbanking_internal_libs) $(gwenhywfar_libs)
qif_la_LDFLAGS = -no-undefined @STRIPALL@ -module -omit-version

# Add the linker flags for windows resource file
qif_la_DEPENDENCIES = \
 $(aqbanking_internal_libs)
# if IS_WINDOWS
if IS_WINDOWS
qif_la_DEPENDENCIES += ressource.o
qif_la_LDFLAGS += -Wl,ressource.o
ressource.o: ressource.rc
	$(RC) -o $@ $<
endif
# IS_WINDOWS
# End of linker flags for windows resource file


sources:
	for f in $(qif_la_SOURCES); do \
	  echo $(subdir)/$$f >>$(top_srcdir)/i18nsources; \
	done
