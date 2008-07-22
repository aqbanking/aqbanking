SUBDIRS=profiles

INCLUDES = -I$(top_builddir)/headers -I$(top_srcdir)/src/libs/aqbanking $(all_includes)
EXTRA_DIST=eriformat.xml

noinst_LTLIBRARIES=libio_eri2.la
noinst_HEADERS=eri2_p.h
libio_eri2_la_SOURCES=eri2.c


imexporterplugindir = $(aqbanking_plugindir)/imexporters
imexporterplugin_LTLIBRARIES=eri2.la
imexporterplugin_DATA=eri2.xml

imexporterdatadir=$(aqbanking_pkgdatadir)/imexporters/eri2
dist_imexporterdata_DATA=eriformat.xml

eri2_la_SOURCES=dummy.c
eri2_la_LIBADD=libio_eri2.la $(aqbanking_internal_libs) $(gwenhywfar_libs) 
eri2_la_LDFLAGS = -no-undefined @STRIPALL@ -module -avoid-version

# Add the linker flags for windows resource file
eri2_la_DEPENDENCIES = libio_eri2.la $(aqbanking_internal_libs)
# if IS_WINDOWS
if IS_WINDOWS
eri2_la_DEPENDENCIES += ressource.o
eri2_la_LDFLAGS += -Wl,ressource.o
ressource.o: ressource.rc
	$(RC) -o $@ $<
endif
# IS_WINDOWS
# End of linker flags for windows resource file


sources:
	for f in $(libio_eri2_la_SOURCES); do \
	  echo $(subdir)/$$f >>$(top_srcdir)/i18nsources; \
	done
