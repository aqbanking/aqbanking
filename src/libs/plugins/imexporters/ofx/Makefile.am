SUBDIRS=parser profiles

INCLUDES = -I$(top_srcdir)/src/libs \
  -I$(top_builddir)/src/libs \
  $(gwenhywfar_includes) -Iparser -I$(srcdir)/parser

AM_CFLAGS=@visibility_cflags@

#EXTRA_DIST=README

noinst_HEADERS=ofx_p.h ofx.h

imexporterplugindir = $(aqbanking_plugindir)/imexporters
noinst_LTLIBRARIES=libabimexporters_ofx.la
imexporterplugin_DATA=ofx.xml

libabimexporters_ofx_la_SOURCES=ofx.c
libabimexporters_ofx_la_LIBADD=parser/libofxparser.la
  


sources:
	for d in $(SUBDIRS); do \
	  $(MAKE) -C $$d sources; \
	done
	for f in $(libabimexporters_ofx_la_SOURCES); do \
	  echo $(subdir)/$$f >>$(top_srcdir)/i18nsources; \
	done
	for f in $(imexporterplugin_DATA); do \
	  echo $(subdir)/$$f >>$(top_srcdir)/pdsources; \
	done


cppcheck:
	for f in $(libabimexporters_ofx_la_SOURCES); do \
	  cppcheck --force $$f ; \
	done
	for d in $(SUBDIRS); do \
	  $(MAKE) -C $$d cppcheck; \
	done


