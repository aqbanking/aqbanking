INCLUDES=-I$(aqbanking_symlinkdir) @all_includes@

AM_CFLAGS=-DBUILDING_AQBANKING @visibility_cflags@

noinst_LTLIBRARIES=libtypes.la

typefiles=\
  transaction.xml \
  pin.xml \
  bankinfo.xml \
  eutransferinfo.xml \
  user.xml \
  cellphone.xml \
  security.xml \
  message.xml

EXTRA_DIST=$(typefiles)

noinst_HEADERS=\
  accstatus_p.h \
  accstatus_l.h \
  balance_p.h \
  bankinfo_l.h \
  bankinfo_p.h \
  eutransferinfo_l.h \
  eutransferinfo_p.h \
  bankinfoservice_l.h \
  bankinfoservice_p.h \
  pin_l.h \
  pin_p.h \
  textkeydescr_l.h \
  textkeydescr_p.h \
  transaction_l.h \
  transaction_p.h \
  transactionlimits_l.h \
  transactionlimits_p.h \
  user_l.h \
  user_p.h \
  userfns_l.h \
  value_p.h \
  cellphoneproduct_l.h \
  cellphoneproduct_p.h \
  security_l.h \
  security_p.h \
  message_l.h \
  message_p.h


iheaderdir=@aqbanking_headerdir_am@/aqbanking
iheader_HEADERS=\
  accstatus.h \
  balance.h \
  bankinfo.h \
  bankinfoservice.h \
  textkeydescr.h \
  transaction.h \
  transactionfns.h \
  transactionlimits.h \
  eutransferinfo.h \
  user.h \
  userfns.h \
  value.h \
  cellphoneproduct.h \
  security.h \
  message.h

libtypes_la_SOURCES=\
  accstatus.c \
  balance.c \
  bankinfo.c \
  bankinfoservice.c \
  eutransferinfo.c \
  pin.c \
  textkeydescr.c \
  transaction.c \
  transactionfns.c \
  transactionlimits.c \
  user.c \
  userfns.c \
  value.c \
  cellphoneproduct.c \
  security.c \
  message.c


types: $(typefiles)
	$(gwenhywfar_bindir)/typemaker -d AQBANKING_API $(typefiles)


sources:
	for f in $(libtypes_la_SOURCES); do \
	  echo $(subdir)/$$f >>$(top_srcdir)/i18nsources; \
	done

