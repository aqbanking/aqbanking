INCLUDES = -I$(aqbanking_symlinkdir) $(all_includes) $(ktoblzcheck_includes)

AM_CFLAGS=@visibility_cflags@

noinst_HEADERS=de.h

bankinfoplugindir = $(aqbanking_plugindir)/bankinfo
noinst_LTLIBRARIES=libbankinfo_de.la
bankinfoplugin_DATA=de.xml

libbankinfo_de_la_SOURCES=de.c
libbankinfo_de_la_LIBADD= ../generic/libbankinfo_generic.la


sources:
	for f in $(libbankinfo_de_la_SOURCES); do \
	  echo $(subdir)/$$f >>$(top_srcdir)/i18nsources; \
	done
	for f in $(bankinfoplugin_DATA); do \
	  echo $(subdir)/$$f >>$(top_srcdir)/pdsources; \
	done


cppcheck:
	for f in $(libbankinfo_de_la_SOURCES); do \
	  cppcheck --force $$f ; \
	done
	for d in $(SUBDIRS); do \
	  $(MAKE) -C $$d cppcheck; \
	done


