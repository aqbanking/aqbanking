
AM_CFLAGS=-DBUILDING_AQHBCI @visibility_cflags@

INCLUDES = -I$(srcdir) -I$(srcdir)/.. -I$(srcdir)/../banking \
  -I$(top_builddir)/headers $(all_includes) \
  -I$(top_srcdir)/src/libs/aqbanking


EXTRA_DIST=msgcrypt.inc

DEFS += -DAH_XMLDATADIR=\"@aqhbci_data@/xml\"

typefiles=mediumctx.xml


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
 medium_l.h \
 medium_p.h \
 mediumheader_p.h \
 mediumctx_l.h \
 mediumctx_p.h \
 message_l.h \
 message_p.h \
 msgengine_l.h \
 msgengine_p.h \
 user_p.h \
 user_l.h

iheaderdir=${includedir}/aqhbci
iheader_HEADERS=\
 account.h \
 medium.h \
 mediumctx.h \
 msgengine.h \
 user.h

libhbcimsg_la_SOURCES=\
 account.c \
 bpd.c \
 dialog.c \
 hbci.c \
 hbci-updates.c \
 medium.c \
 mediumctx.c \
 message.c \
 msgengine.c \
 user.c

sources:
	for f in $(libhbcimsg_la_SOURCES); do \
	  echo $(subdir)/$$f >>$(top_srcdir)/i18nsources; \
	done
	for d in $(SUBDIRS); do \
	  make -C $$d sources; \
	done


types: $(typefiles)
	typemaker -d AQHBCI_API $(typefiles)

