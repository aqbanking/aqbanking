SUBDIRS=profiles

INCLUDES = -I$(top_srcdir)/src/libs/aqbanking -I$(aqbanking_symlinkdir) $(all_includes)

EXTRA_DIST=README


noinst_HEADERS=swift_p.h


imexporterplugindir = $(aqbanking_plugindir)/imexporters
imexporterplugin_LTLIBRARIES=swift.la
imexporterplugin_DATA=swift.xml 

swift_la_SOURCES=swift.c
swift_la_LIBADD=$(aqbanking_internal_libs) $(gwenhywfar_libs) 
swift_la_LDFLAGS = -no-undefined @STRIPALL@ -module -avoid-version


sources:
	for f in $(swift_la_SOURCES); do \
	  echo $(subdir)/$$f >>$(top_srcdir)/i18nsources; \
	done
	for f in $(imexporterplugin_DATA); do \
	  echo $(subdir)/$$f >>$(top_srcdir)/pdsources; \
	done
