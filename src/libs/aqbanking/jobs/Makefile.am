INCLUDES = -I$(top_srcdir) \
  $(all_includes) \
  -I$(srcdir)/.. \
  -I$(srcdir)/../types

noinst_LTLIBRARIES=libjobs.la

noinst_HEADERS=\
 jobgettransactions_p.h \
 jobgetbalance_p.h \
 jobgettransactions_p.h \
 jobsingletransfer_p.h \
 jobsingledebitnote_p.h \
 jobeutransfer_p.h



iheaderdir=${includedir}/aqbanking
iheader_HEADERS= \
 jobgetbalance.h \
 jobgetbalance_be.h \
 jobgetbalance_l.h \
 jobgettransactions.h \
 jobgettransactions_be.h \
 jobgettransactions_l.h \
 jobsingletransfer.h \
 jobsingletransfer_be.h \
 jobsingletransfer_l.h \
 jobsingledebitnote.h \
 jobsingledebitnote_be.h \
 jobsingledebitnote_l.h \
 jobeutransfer.h \
 jobeutransfer_l.h \
 jobeutransfer_be.h

libjobs_la_SOURCES=\
 jobgetbalance.c \
 jobgettransactions.c \
 jobsingletransfer.c \
 jobsingledebitnote.c \
 jobeutransfer.c


sources:
	for f in $(libjobs_la_SOURCES); do \
	  echo $(subdir)/$$f >>$(top_srcdir)/i18nsources; \
	done


