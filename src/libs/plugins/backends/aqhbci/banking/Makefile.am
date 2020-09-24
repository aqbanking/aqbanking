
AM_CFLAGS=-DBUILDING_AQBANKING @visibility_cflags@

AM_CPPFLAGS = -I$(top_srcdir)/src/libs -I$(top_builddir)/src/libs $(gwenhywfar_includes) \
  -I$(srcdir)/../.. \
  -I$(srcdir)/../msglayer -I$(srcdir)/../joblayer \
  -I$(srcdir)/../applayer -I$(srcdir)/.. \
  -I$(srcdir)/../tan \
  -I$(srcdir)/../ajobs -I$(srcdir)/../admjobs \
  -I$(srcdir)/../dialogs  -I$(srcdir)/../control

localedir = $(datadir)/locale
DEFS += -DLOCALEDIR=\"$(localedir)\" \
  -DAQHBCI_PLUGINS=\"$(aqhbci_plugindir)\"

noinst_LTLIBRARIES=libhbcibanking.la

noinst_HEADERS=\
 provider_l.h \
 provider_p.h \
 provider.h \
 provider_accspec.h \
 provider_dialogs.h \
 provider_iniletter.h \
 provider_job.h \
 provider_keys.h \
 provider_online.h \
 provider_sendcmd.h \
 provider_tan.c \
 provider_update.h \
 user.h \
 user_p.h \
 user_l.h \
 account.h \
 account_l.h \
 account_p.h


#iheaderdir=@aqbanking_headerdir_am@/aqhbci
#iheader_HEADERS=

libhbcibanking_la_SOURCES=\
 provider.c \
 provider_accspec.c \
 provider_dialogs.c \
 provider_iniletter.c \
 provider_job.c \
 provider_keys.c \
 provider_online.c \
 provider_sendcmd.c \
 provider_tan.c \
 provider_update.c \
 account.c \
 user.c


EXTRA_DIST=\
  provider_dtazv.c



sources:
	for f in $(libhbcibanking_la_SOURCES); do \
	  echo $(subdir)/$$f >>$(top_srcdir)/i18nsources; \
	done
	for d in $(SUBDIRS); do \
	  $(MAKE) -C $$d sources; \
	done


cppcheck:
	for f in $(libhbcibanking_la_SOURCES); do \
	  cppcheck --force $$f ; \
	done
	for d in $(SUBDIRS); do \
	  $(MAKE) -C $$d cppcheck; \
	done


