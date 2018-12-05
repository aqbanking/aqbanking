
AM_CFLAGS=-DBUILDING_AQHBCI @visibility_cflags@

INCLUDES = -I$(aqbanking_symlinkdir) $(gwenhywfar_includes) \
  -I$(srcdir)/../msglayer -I$(srcdir)/../joblayer \
  -I$(srcdir)/../applayer -I$(srcdir)/../ajobs  -I$(srcdir)/.. \
  -I$(srcdir)/../dialogs

localedir = $(datadir)/locale
DEFS += -DLOCALEDIR=\"$(localedir)\" \
  -DAQHBCI_PLUGINS=\"$(aqhbci_plugindir)\" \
  -DXMLDATA_DIR=\"$(aqbanking_pkgdatadir)/backends/aqhbci\"

noinst_LTLIBRARIES=libhbcicontrol.la

noinst_HEADERS=\
 globals_l.h \
 control_l.h


iheaderdir=@aqbanking_headerdir_am@/aqhbci
#iheader_HEADERS=

libhbcicontrol_la_SOURCES=\
 main.c \
 addaccount.c \
 addsubaccountflags.c \
 addsubuserflags.c \
 adduser.c \
 changepin.c \
 createkeys.c \
 delaccount.c \
 deluser.c \
 getaccounts.c \
 getaccsepa.c \
 getcert.c \
 getitanmodes.c \
 getkeys.c \
 getsysid.c \
 iniletter.c \
 listaccounts.c \
 listitanmodes.c \
 listusers.c \
 mkpinlist.c \
 logfile.c \
 resetkeys.c \
 sendkeys.c \
 sethbciversion.c \
 setitanmode.c \
 setmaxtransfers.c \
 setsepaprofile.c \
 settanmediumid.c \
 test.c





#EXTRA_DIST=



sources:
	for f in $(libhbcicontrol_la_SOURCES); do \
	  echo $(subdir)/$$f >>$(top_srcdir)/i18nsources; \
	done
	for d in $(SUBDIRS); do \
	  $(MAKE) -C $$d sources; \
	done

