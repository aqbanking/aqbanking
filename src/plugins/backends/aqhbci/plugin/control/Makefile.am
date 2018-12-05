
AM_CFLAGS=-DBUILDING_AQHBCI @visibility_cflags@

INCLUDES = -I$(aqbanking_symlinkdir) $(gwenhywfar_includes) \
  -I$(srcdir)/../msglayer -I$(srcdir)/../joblayer \
  -I$(srcdir)/../applayer -I$(srcdir)/../ajobs  -I$(srcdir)/.. \
  -I$(srcdir)/../dialogs

localedir = $(datadir)/locale
DEFS += -DLOCALEDIR=\"$(localedir)\" \
  -DAQHBCI_PLUGINS=\"$(aqhbci_plugindir)\"

noinst_LTLIBRARIES=libhbcicontrol.la

noinst_HEADERS=\
 globals_l.h \
 control_l.h


iheaderdir=@aqbanking_headerdir_am@/aqhbci
#iheader_HEADERS=

libhbcicontrol_la_SOURCES=\
 main.c \
 mkpinlist.c



#EXTRA_DIST=



sources:
	for f in $(libhbcicontrol_la_SOURCES); do \
	  echo $(subdir)/$$f >>$(top_srcdir)/i18nsources; \
	done
	for d in $(SUBDIRS); do \
	  $(MAKE) -C $$d sources; \
	done

