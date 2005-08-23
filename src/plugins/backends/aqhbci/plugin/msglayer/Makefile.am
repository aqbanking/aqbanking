INCLUDES = -I$(top_builddir)/headers -I$(top_srcdir)/src/libs/aqbanking \
  $(all_includes) -I$(srcdir)/..

DEFS += -DAH_XMLDATADIR=\"@aqhbci_data@/xml\"

typefiles=mediumctx.xml


noinst_LTLIBRARIES=libhbcimsg.la

noinst_HEADERS=\
 account_l.h \
 account_p.h \
 bank_l.h \
 bank_p.h \
 bpd_p.h \
 connection_p.h \
 connectionhbci_p.h \
 customer_l.h \
 customer_p.h \
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
 msgengine_p.h \
 objectref_p.h \
 user_p.h \
 user_l.h

iheaderdir=${includedir}/aqhbci
iheader_HEADERS=\
 account.h \
 bank.h \
 bpd.h \
 connection.h \
 connectionhbci.h \
 customer.h \
 dialog.h \
 hbci.h \
 medium.h \
 mediumctx.h \
 mediumheader.h \
 message.h \
 msgengine.h \
 objectref.h \
 user.h

libhbcimsg_la_SOURCES=\
 account.c \
 bank.c \
 bpd.c \
 connection.c \
 connectionhbci.c \
 customer.c \
 dialog.c \
 hbci.c \
 hbci-updates.c \
 medium.c \
 mediumheader.c \
 mediumctx.c \
 message.c \
 msgcrypt.c \
 msgengine.c \
 objectref.c \
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

