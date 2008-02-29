INCLUDES = -I$(top_builddir)/headers $(all_includes)

crypttokenplugindir = $(gwenhywfar_plugindir)/ct

crypttokenplugin_LTLIBRARIES=rdh2.la
crypttokenplugin_DATA=rdh2.xml
noinst_HEADERS=\
  tlv_l.h tlv_p.h

rdh2_la_SOURCES=\
  tlv.c
rdh2_la_LIBADD=$(gwenhywfar_libs)

rdh2_la_LDFLAGS = -no-undefined $(STRIPALL) \
  -module -omit-version


sources:
	for f in $(rdh2_la_SOURCES); do \
	  echo $(subdir)/$$f >>$(top_srcdir)/i18nsources; \
	done



built_sources:
	for d in $(SUBDIRS); do \
	  $(MAKE) -C $$d built_sources; \
	done ;
