SUBDIRS=profiles

INCLUDES = -I$(top_builddir)/headers -I$(top_srcdir)/src/libs/aqbanking $(all_includes)
EXTRA_DIST=README

imexporterplugindir = $(aqbanking_plugindir)/imexporters
imexporterplugin_LTLIBRARIES=ctxfile.la
imexporterplugin_DATA=ctxfile.xml

noinst_LTLIBRARIES=libio_ctxfile.la
libio_ctxfile_la_SOURCES=ctxfile.c
noinst_HEADERS=ctxfile_p.h


ctxfile_la_SOURCES=dummy.c
ctxfile_la_LIBADD=libio_ctxfile.la $(aqbanking_internal_libs) $(gwenhywfar_libs) 
ctxfile_la_LDFLAGS = -no-undefined @STRIPALL@ -module -avoid-version

# Add the linker flags for windows resource file
ctxfile_la_DEPENDENCIES = libio_ctxfile.la $(aqbanking_internal_libs)
# if IS_WINDOWS
if IS_WINDOWS
ctxfile_la_DEPENDENCIES += ressource.o
ctxfile_la_LDFLAGS += -Wl,ressource.o
ressource.o: ressource.rc
	$(RC) -o $@ $<
endif
# IS_WINDOWS
# End of linker flags for windows resource file


sources:
	for f in $(libio_ctxfile_la_SOURCES); do \
	  echo $(subdir)/$$f >>$(top_srcdir)/i18nsources; \
	done
