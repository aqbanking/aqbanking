SUBDIRS=profiles

INCLUDES = -I$(top_srcdir)/src/libs/aqbanking -I$(top_builddir)/headers $(all_includes)

EXTRA_DIST=README


noinst_LTLIBRARIES=libio_swift.la
noinst_HEADERS=swift_p.h
libio_swift_la_SOURCES=swift.c


imexporterplugindir = $(aqbanking_plugindir)/imexporters
imexporterplugin_LTLIBRARIES=swift.la
imexporterplugin_DATA=swift.xml 

swift_la_SOURCES=dummy.c
swift_la_LIBADD=libio_swift.la $(aqbanking_internal_libs) $(gwenhywfar_libs) 
swift_la_LDFLAGS = -no-undefined @STRIPALL@ -module -avoid-version

# Add the linker flags for windows resource file
swift_la_DEPENDENCIES = libio_swift.la $(aqbanking_internal_libs)
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
	for f in $(libio_swift_la_SOURCES); do \
	  echo $(subdir)/$$f >>$(top_srcdir)/i18nsources; \
	done
