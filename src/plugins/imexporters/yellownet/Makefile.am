SUBDIRS=profiles

INCLUDES = -I$(top_builddir)/headers -I$(top_srcdir)/src/libs/aqbanking $(all_includes)
#EXTRA_DIST=

noinst_LTLIBRARIES=libio_yellownet.la
noinst_HEADERS=yellownet_p.h
libio_yellownet_la_SOURCES=yellownet.c

imexporterplugindir = $(aqbanking_plugindir)/imexporters
imexporterplugin_LTLIBRARIES=yellownet.la
imexporterplugin_DATA=yellownet.xml

yellownet_la_SOURCES=dummy.c
yellownet_la_LIBADD=libio_yellownet.la $(aqbanking_internal_libs) $(gwenhywfar_libs) 
yellownet_la_LDFLAGS = -no-undefined @STRIPALL@ -module -avoid-version

# Add the linker flags for windows resource file
yellownet_la_DEPENDENCIES = libio_yellownet.la $(aqbanking_internal_libs)
# if IS_WINDOWS
if IS_WINDOWS
yellownet_la_DEPENDENCIES += ressource.o
yellownet_la_LDFLAGS += -Wl,ressource.o
ressource.o: ressource.rc
	$(RC) -o $@ $<
endif
# IS_WINDOWS
# End of linker flags for windows resource file


sources:
	for f in $(libio_yellownet_la_SOURCES); do \
	  echo $(subdir)/$$f >>$(top_srcdir)/i18nsources; \
	done
