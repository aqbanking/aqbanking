SUBDIRS=profiles

INCLUDES = -I$(top_srcdir)/src/libs \
  -I$(top_builddir)/src/libs \
  $(gwenhywfar_includes)

AM_CFLAGS=@visibility_cflags@

#EXTRA_DIST=

noinst_HEADERS=yellownet_p.h yellownet.h

imexporterplugindir = $(aqbanking_plugindir)/imexporters
noinst_LTLIBRARIES=libabimexporters_yellownet.la
imexporterplugin_DATA=yellownet.xml

libabimexporters_yellownet_la_SOURCES=yellownet.c


sources:
	for f in $(libabimexporters_yellownet_la_SOURCES); do \
	  echo $(subdir)/$$f >>$(top_srcdir)/i18nsources; \
	done
	for f in $(imexporterplugin_DATA); do \
	  echo $(subdir)/$$f >>$(top_srcdir)/pdsources; \
	done

cppcheck:
	for f in $(libabimexporters_yellownet_la_SOURCES); do \
	  cppcheck --force $$f ; \
	done

