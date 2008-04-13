SUBDIRS=profiles

INCLUDES = -I$(top_builddir)/headers -I$(top_srcdir)/src/libs/aqbanking $(all_includes)
EXTRA_DIST=README

imexporterplugindir = $(aqbanking_plugindir)/imexporters
imexporterplugin_LTLIBRARIES=openhbci1.la
imexporterplugin_DATA=openhbci1.xml

noinst_LTLIBRARIES=libio_openhbci1.la
libio_openhbci1_la_SOURCES=openhbci1.c
noinst_HEADERS=openhbci1_p.h


openhbci1_la_SOURCES=dummy.c
openhbci1_la_LIBADD=libio_openhbci1.la $(aqbanking_internal_libs) $(gwenhywfar_libs) 
openhbci1_la_LDFLAGS = -no-undefined @STRIPALL@ -module -omit-version

# Add the linker flags for windows resource file
openhbci1_la_DEPENDENCIES = libio_openhbci1.la $(aqbanking_internal_libs)
# if IS_WINDOWS
if IS_WINDOWS
openhbci1_la_DEPENDENCIES += ressource.o
openhbci1_la_LDFLAGS += -Wl,ressource.o
ressource.o: ressource.rc
	$(RC) -o $@ $<
endif
# IS_WINDOWS
# End of linker flags for windows resource file


sources:
	for f in $(libio_openhbci1_la_SOURCES); do \
	  echo $(subdir)/$$f >>$(top_srcdir)/i18nsources; \
	done
