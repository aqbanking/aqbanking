INCLUDES=-I${top_srcdir} @all_includes@

noinst_LTLIBRARIES=libtypes.la

typefiles=transaction.xml pin.xml bankinfo.xml eutransferinfo.xml

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
  split_l.h \
  split_p.h \
  transaction_l.h \
  transaction_p.h \
  value_p.h


iheaderdir=${includedir}/aqbanking
iheader_HEADERS=\
  accstatus.h \
  balance.h \
  bankinfo.h \
  bankinfoservice.h \
  split.h \
  transaction.h \
  eutransferinfo.h \
  value.h

libtypes_la_SOURCES=\
  accstatus.c \
  balance.c \
  bankinfo.c \
  bankinfoservice.c \
  eutransferinfo.c \
  pin.c \
  split.c \
  transaction.c \
  value.c


types: $(typefiles)
	typemaker -d AQBANKING_API $(typefiles)


sources:
	for f in $(libtypes_la_SOURCES); do \
	  echo $(subdir)/$$f >>$(top_srcdir)/i18nsources; \
	done

