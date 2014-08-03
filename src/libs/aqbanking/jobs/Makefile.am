INCLUDES = -I$(aqbanking_symlinkdir) \
  $(all_includes) \
  -I$(srcdir)/.. \
  -I$(srcdir)/../types

AM_CFLAGS=-DBUILDING_AQBANKING @visibility_cflags@

noinst_LTLIBRARIES=libjobs.la

noinst_HEADERS=\
 jobgettransactions_p.h \
 jobgetstandingorders_p.h \
 jobgetdatedtransfers_p.h \
 jobgetbalance_p.h \
 jobeutransfer_p.h \
 jobcreatesto_l.h \
 jobgetbalance_l.h \
 jobgettransactions_l.h \
 jobgetstandingorders_l.h \
 jobsingletransfer_l.h \
 jobsingledebitnote_l.h \
 jobsepatransfer_l.h \
 jobsepadebitnote_l.h \
 jobgetdatedtransfers_l.h \
 jobmodifysto_l.h \
 jobdeletesto_l.h \
 jobcreatedatedtransfer_l.h \
 jobmodifydatedtransfer_l.h \
 jobdeletedatedtransfer_l.h \
 jobinternaltransfer_l.h \
 jobeutransfer_l.h \
 jobloadcellphone_l.h \
 jobloadcellphone_p.h \
 jobsepacreatesto.h



iheaderdir=@aqbanking_headerdir_am@/aqbanking
iheader_HEADERS= \
 jobgetbalance.h \
 jobgetbalance_be.h \
 jobgettransactions.h \
 jobgettransactions_be.h \
 jobgetstandingorders.h \
 jobgetstandingorders_be.h \
 jobsingletransfer.h \
 jobsingletransfer_be.h \
 jobsingledebitnote.h \
 jobsingledebitnote_be.h \
 jobsepatransfer.h \
 jobsepatransfer_be.h \
 jobsepadebitnote.h \
 jobsepadebitnote_be.h \
 jobsepaflashdebitnote.h \
 jobgetdatedtransfers.h \
 jobgetdatedtransfers_be.h \
 jobcreatesto.h \
 jobcreatesto_be.h \
 jobmodifysto.h \
 jobmodifysto_be.h \
 jobdeletesto.h \
 jobdeletesto_be.h \
 jobcreatedatedtransfer.h \
 jobcreatedatedtransfer_be.h \
 jobmodifydatedtransfer.h \
 jobmodifydatedtransfer_be.h \
 jobdeletedatedtransfer.h \
 jobdeletedatedtransfer_be.h \
 jobinternaltransfer.h \
 jobinternaltransfer_be.h \
 jobeutransfer.h \
 jobeutransfer_be.h \
 jobloadcellphone.h \
 jobloadcellphone_be.h \
 jobsepagetstandingorders.h

libjobs_la_SOURCES=\
 jobgetbalance.c \
 jobgettransactions.c \
 jobgetstandingorders.c \
 jobgetdatedtransfers.c \
 jobsingletransfer.c \
 jobsingledebitnote.c \
 jobsepatransfer.c \
 jobsepadebitnote.c \
 jobsepaflashdebitnote.c \
 jobcreatesto.c \
 jobmodifysto.c \
 jobdeletesto.c \
 jobcreatedatedtransfer.c \
 jobmodifydatedtransfer.c \
 jobdeletedatedtransfer.c \
 jobinternaltransfer.c \
 jobeutransfer.c \
 jobloadcellphone.c \
 jobsepacreatesto.c \
 jobsepagetstandingorders.c



sources:
	for f in $(libjobs_la_SOURCES); do \
	  echo $(subdir)/$$f >>$(top_srcdir)/i18nsources; \
	done


