INCLUDES = -I$(top_srcdir) \
  $(all_includes) \
  -I$(srcdir)/.. \
  -I$(srcdir)/../types

noinst_LTLIBRARIES=libjobs.la

noinst_HEADERS=\
 jobgettransactions_p.h \
 jobgetstandingorders_p.h \
 jobgetdatedtransfers_p.h \
 jobgetbalance_p.h \
 jobtransferbase_p.h \
 jobeutransfer_p.h



iheaderdir=${includedir}/aqbanking
iheader_HEADERS= \
 jobgetbalance.h \
 jobgetbalance_be.h \
 jobgetbalance_l.h \
 jobgettransactions.h \
 jobgettransactions_be.h \
 jobgettransactions_l.h \
 jobgetstandingorders.h \
 jobgetstandingorders_be.h \
 jobgetstandingorders_l.h \
 jobtransferbase_l.h \
 jobsingletransfer.h \
 jobsingletransfer_be.h \
 jobsingledebitnote.h \
 jobsingledebitnote_be.h \
 jobgetdatedtransfers.h \
 jobgetdatedtransfers_be.h \
 jobgetdatedtransfers_l.h \
 jobcreatesto.h \
 jobcreatesto_be.h \
 jobmodifysto.h \
 jobmodifysto_be.h \
 jobdeletesto.h \
 jobdeletesto_be.h \
 jobeutransfer.h \
 jobeutransfer_l.h \
 jobeutransfer_be.h

libjobs_la_SOURCES=\
 jobgetbalance.c \
 jobgettransactions.c \
 jobgetstandingorders.c \
 jobgetdatedtransfers.c \
 jobtransferbase.c \
 jobsingletransfer.c \
 jobsingledebitnote.c \
 jobcreatesto.c \
 jobmodifysto.c \
 jobdeletesto.c \
 jobeutransfer.c


sources:
	for f in $(libjobs_la_SOURCES); do \
	  echo $(subdir)/$$f >>$(top_srcdir)/i18nsources; \
	done


