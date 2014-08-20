
AM_CFLAGS=-DBUILDING_AQHBCI @visibility_cflags@

INCLUDES = -I$(aqbanking_symlinkdir) $(gwenhywfar_includes) \
  -I$(srcdir)/../msglayer -I$(srcdir)/../joblayer \
  -I$(srcdir)/../applayer -I$(srcdir)/../ajobs  -I$(srcdir)/.. \
  -I$(srcdir)/../dialogs

localedir = $(datadir)/locale
DEFS += -DLOCALEDIR=\"$(localedir)\" \
  -DAQHBCI_PLUGINS=\"$(aqhbci_plugindir)\"

noinst_LTLIBRARIES=libhbcibanking.la

noinst_HEADERS=\
 provider_l.h \
 provider_p.h


iheaderdir=@aqbanking_headerdir_am@/aqhbci
iheader_HEADERS=\
 provider.h

libhbcibanking_la_SOURCES=\
 provider.c

sources:
	for f in $(libhbcibanking_la_SOURCES); do \
	  echo $(subdir)/$$f >>$(top_srcdir)/i18nsources; \
	done
	for d in $(SUBDIRS); do \
	  $(MAKE) -C $$d sources; \
	done

