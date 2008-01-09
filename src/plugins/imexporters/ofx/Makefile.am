SUBDIRS=parser profiles

# Include the source code files here as well. It doesnt hurt to
# include them twice in the dist target.
EXTRA_DIST=\
  ofx.c \
  ofx-internal.c ofx-internal_p.h \
  ofx.xml.in


INCLUDES = -I$(top_builddir)/headers $(gwenhywfar_includes) -Iparser
#EXTRA_DIST=README

imexporterplugindir = $(aqbanking_plugindir)/imexporters

imexporterplugin_LTLIBRARIES=ofx.la
imexporterplugin_DATA=ofx.xml

noinst_HEADERS=\
 ofx-internal_p.h

ofx_la_SOURCES=\
 ofx.c

ofx_la_LIBADD=\
  parser/libofxparser.la \
  $(aqbanking_internal_libs) $(gwenhywfar_libs) 

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
	for d in $(SUBDIRS); do \
	  $(MAKE) -C $$d sources; \
	done
	for f in $(ofx_la_SOURCES); do \
	  echo $(subdir)/$$f >>$(top_srcdir)/i18nsources; \
	done


