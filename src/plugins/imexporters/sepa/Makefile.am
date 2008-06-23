SUBDIRS=profiles

INCLUDES = -I$(top_srcdir)/src/libs/aqbanking -I$(top_builddir)/headers $(all_includes)

extra_sources=\
  sepa_exp_ccm.c

EXTRA_DIST=README $(extra_sources)

noinst_HEADERS=sepa_p.h
noinst_LTLIBRARIES=libio_sepa.la
libio_sepa_la_SOURCES=sepa.c

imexporterplugindir = $(aqbanking_plugindir)/imexporters
imexporterplugin_LTLIBRARIES=sepa.la
imexporterplugin_DATA=sepa.xml

sepa_la_SOURCES=dummy.c
sepa_la_LIBADD=libio_sepa.la $(aqbanking_internal_libs) $(gwenhywfar_libs) 
sepa_la_LDFLAGS = -no-undefined @STRIPALL@ -module -omit-version

# Add the linker flags for windows resource file
sepa_la_DEPENDENCIES = libio_sepa.la $(aqbanking_internal_libs)
# if IS_WINDOWS
if IS_WINDOWS
sepa_la_DEPENDENCIES += ressource.o
sepa_la_LDFLAGS += -Wl,ressource.o
ressource.o: ressource.rc
	$(RC) -o $@ $<
endif
# IS_WINDOWS
# End of linker flags for windows resource file


sources:
	for f in $(libio_sepa_la_SOURCES); do \
	  echo $(subdir)/$$f >>$(top_srcdir)/i18nsources; \
	done
