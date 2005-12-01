SUBDIRS=profiles

INCLUDES = -I$(top_builddir)/headers $(all_includes)
#EXTRA_DIST=README

abs_top_builddir=$(shell (cd $(top_builddir) && pwd) )

imexporterplugindir = $(aqbanking_plugindir)/imexporters

imexporterplugin_LTLIBRARIES=eri.la
if IS_WINDOWS
imexporterplugin_DATA=eri.xml 
else
imexporterplugin_DATA=eri.xml
endif

noinst_HEADERS=\
 eri_p.h

eri_la_SOURCES=\
 eri.c

eri_la_LIBADD=$(aqbanking_internal_libs) $(gwenhywfar_libs) 
eri_la_LDFLAGS = -no-undefined @STRIPALL@ -module -omit-version

# Add the linker flags for windows resource file
eri_la_DEPENDENCIES = \
 $(aqbanking_internal_libs)
# if IS_WINDOWS
if IS_WINDOWS
eri_la_DEPENDENCIES += ressource.o
eri_la_LDFLAGS += -Wl,ressource.o
ressource.o: ressource.rc
	$(RC) -o $@ $<
endif
# IS_WINDOWS
# End of linker flags for windows resource file


sources:
	for f in $(eri_la_SOURCES); do \
	  echo $(subdir)/$$f >>$(top_srcdir)/i18nsources; \
	done
