SUBDIRS=profiles

# Include the source code files here as well. It doesnt hurt to
# include them twice in the dist target.
EXTRA_DIST=ofx.c ofx_p.h ofx.xml.in

#####
# The complete rest of the file is used only if HAVE_OFX is enabled.
#####
if WITH_OFX_PLUGIN

INCLUDES = -I$(top_builddir)/headers $(libofx_includes) $(gwenhywfar_includes)
#EXTRA_DIST=README

imexporterplugindir = $(aqbanking_plugindir)/imexporters

imexporterplugin_LTLIBRARIES=ofx.la
imexporterplugin_DATA=ofx.xml

noinst_HEADERS=\
 ofx_p.h

ofx_la_SOURCES=\
 ofx.c

ofx_la_LIBADD=$(aqbanking_internal_libs) $(libofx_libs) $(gwenhywfar_libs) 
ofx_la_LDFLAGS = -no-undefined @STRIPALL@ \
  -module -omit-version

# Add the linker flags for windows resource file
ofx_la_DEPENDENCIES = \
 $(aqbanking_internal_libs)
# if IS_WINDOWS
if IS_WINDOWS
ofx_la_DEPENDENCIES += ressource.o
ofx_la_LDFLAGS += -Wl,ressource.o
ressource.o: ressource.rc
	$(RC) -o $@ $<
endif
# IS_WINDOWS
# End of linker flags for windows resource file


sources:
	for f in $(ofx_la_SOURCES); do \
	  echo $(subdir)/$$f >>$(top_srcdir)/i18nsources; \
	done


else
sources:

endif
#####
# WITH_OFX_PLUGIN
#####
