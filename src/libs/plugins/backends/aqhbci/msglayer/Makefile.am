
AM_CFLAGS=-DBUILDING_AQHBCI @visibility_cflags@

INCLUDES = -I$(srcdir) -I$(srcdir)/.. -I$(srcdir)/../banking \
  -I$(srcdir)/../applayer \
  -I$(srcdir)/../joblayer \
  -I$(aqbanking_symlinkdir) $(all_includes) \
  -I$(top_srcdir)/src/libs/aqbanking


EXTRA_DIST=\
  msgcrypt.c \
  msgcrypt_ddv.c \
  msgcrypt_rxh_common.c \
  msgcrypt_pintan.c \
  dialog_hbci.c \
  dialog_https.c

DEFS+=-DAH_XMLDATADIR=\"$(aqbanking_data_searchdir)/aqbanking/backends/aqhbci\"


noinst_LTLIBRARIES=libhbcimsg.la

noinst_HEADERS=\
 account_p.h \
 account_l.h \
 bpd_l.h \
 bpd_p.h \
 dialog_l.h \
 dialog_p.h \
 hbci_l.h \
 hbci_p.h \
 hbci-updates_l.h \
 hbci-updates_p.h \
 message_l.h \
 message_p.h \
 msgengine_l.h \
 msgengine_p.h \
 user_p.h \
 user_l.h \
 account.h \
 msgengine.h \
 user.h

#iheaderdir=@aqbanking_headerdir_am@/aqhbci
#iheader_HEADERS=

libhbcimsg_la_SOURCES=\
 account.c \
 bpd.c \
 dialog.c \
 hbci.c \
 hbci-updates.c \
 message.c \
 msgengine.c \
 user.c

sources:
	for f in $(libhbcimsg_la_SOURCES) $(EXTRA_DIST); do \
	  echo $(subdir)/$$f >>$(top_srcdir)/i18nsources; \
	done
	for d in $(SUBDIRS); do \
	  $(MAKE) -C $$d sources; \
	done


types: $(typefiles)
	typemaker -d AQHBCI_API $(typefiles)



cppcheck:
	for f in $(libhbcimsg_la_SOURCES); do \
	  cppcheck --force $$f ; \
	done
	for d in $(SUBDIRS); do \
	  $(MAKE) -C $$d cppcheck; \
	done


