INCLUDES = -I$(top_builddir)/headers $(all_includes) \
  -I$(srcdir)/../msglayer -I$(srcdir)/../joblayer \
  -I$(srcdir)/../applayer -I$(srcdir)/..

localedir = $(datadir)/locale
DEFS += -DLOCALEDIR=\"$(localedir)\" \
  -DAQHBCI_PLUGINS=\"$(aqhbci_plugindir)\"

noinst_LTLIBRARIES=libhbcibanking.la

noinst_HEADERS=\
 jobplugin_p.h \
 provider_p.h \
 wcb_l.h \
 wcb_p.h


iheaderdir=${includedir}/aqhbci
iheader_HEADERS=\
 jobplugin.h \
 provider.h

libhbcibanking_la_SOURCES=\
 jobplugin.c \
 provider.c \
 wcb.c

sources:
	for f in $(libhbcibanking_la_SOURCES); do \
	  echo $(subdir)/$$f >>$(top_srcdir)/i18nsources; \
	done
	for d in $(SUBDIRS); do \
	  make -C $$d sources; \
	done

