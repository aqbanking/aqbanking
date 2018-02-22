INCLUDES = -I$(aqbanking_symlinkdir) \
  $(all_includes) \
  -I$(srcdir)/.. \
  -I$(srcdir)/../types

AM_CFLAGS=-DBUILDING_AQBANKING @visibility_cflags@

noinst_LTLIBRARIES=libjobs.la

noinst_HEADERS=\
 jobgettransactions_p.h \
 jobloadcellphone_p.h

iheaderdir=@aqbanking_headerdir_am@/aqbanking
iheader_HEADERS= \
 jobgetbalance.h \
 jobgettransactions.h \
 jobgettransactions_be.h \
 jobgetstandingorders.h \
 jobsingletransfer.h \
 jobsingledebitnote.h \
 jobsepatransfer.h \
 jobsepadebitnote.h \
 jobsepaflashdebitnote.h \
 jobgetdatedtransfers.h \
 jobcreatesto.h \
 jobmodifysto.h \
 jobdeletesto.h \
 jobcreatedatedtransfer.h \
 jobmodifydatedtransfer.h \
 jobdeletedatedtransfer.h \
 jobinternaltransfer.h \
 jobloadcellphone.h \
 jobloadcellphone_be.h \
 jobsepacreatesto.h \
 jobsepamodifysto.h \
 jobsepadeletesto.h \
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
 jobloadcellphone.c \
 jobsepacreatesto.c \
 jobsepagetstandingorders.c \
 jobsepamodifysto.c \
 jobsepadeletesto.c

sources:
	for f in $(libjobs_la_SOURCES); do \
	  echo $(subdir)/$$f >>$(top_srcdir)/i18nsources; \
	done


