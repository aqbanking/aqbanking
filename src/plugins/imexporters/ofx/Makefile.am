SUBDIRS=parser profiles

# Include the source code files here as well. It doesnt hurt to
# include them twice in the dist target.
EXTRA_DIST=\
  ofx.c \
  ofx-internal.c ofx-internal_p.h \
  ofx.xml.in


INCLUDES = -I$(aqbanking_symlinkdir) $(gwenhywfar_includes) -Iparser -I$(srcdir)/parser
#EXTRA_DIST=README

noinst_HEADERS=ofx-internal_p.h

imexporterplugindir = $(aqbanking_plugindir)/imexporters
imexporterplugin_LTLIBRARIES=ofx.la
imexporterplugin_DATA=ofx.xml

ofx_la_SOURCES=ofx.c
ofx_la_LIBADD = parser/libofxparser.la $(aqbanking_internal_libs) $(gwenhywfar_libs)
ofx_la_LDFLAGS = -no-undefined @STRIPALL@ -module -avoid-version


sources:
	for d in $(SUBDIRS); do \
	  $(MAKE) -C $$d sources; \
	done
	for f in $(ofx_la_SOURCES); do \
	  echo $(subdir)/$$f >>$(top_srcdir)/i18nsources; \
	done
	for f in $(imexporterplugin_DATA); do \
	  echo $(subdir)/$$f >>$(top_srcdir)/pdsources; \
	done

