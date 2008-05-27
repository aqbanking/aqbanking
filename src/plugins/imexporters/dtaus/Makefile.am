SUBDIRS=profiles

INCLUDES = -I$(top_builddir)/headers $(all_includes) -I$(top_srcdir)/src/libs/aqbanking
EXTRA_DIST=README

noinst_LTLIBRARIES=libio_dtaus.la
libio_dtaus_la_SOURCES=dtaus.c
noinst_HEADERS=dtaus_p.h


imexporterplugindir = $(aqbanking_plugindir)/imexporters
imexporterplugin_LTLIBRARIES=dtaus.la
imexporterplugin_DATA=dtaus.xml

dtaus_la_SOURCES=dummy.c
dtaus_la_LIBADD=libio_dtaus.la $(aqbanking_internal_libs) $(gwenhywfar_libs) 
dtaus_la_LDFLAGS = -no-undefined @STRIPALL@ -module -omit-version

# Add the linker flags for windows resource file
dtaus_la_DEPENDENCIES = libio_dtaus.la $(aqbanking_internal_libs)

if IS_WINDOWS
dtaus_la_DEPENDENCIES += ressource.o
dtaus_la_LDFLAGS += -Wl,ressource.o
ressource.o: ressource.rc
	$(RC) -o $@ $<
endif
# End of linker flags for windows resource file


sources:
	for f in $(libio_dtaus_la_SOURCES); do \
	  echo $(subdir)/$$f >>$(top_srcdir)/i18nsources; \
	done
