
AM_CFLAGS=-DBUILDING_AQHBCI @visibility_cflags@

INCLUDES = -I$(top_builddir)/headers $(all_includes) \
  -I$(srcdir)/../joblayer \
  -I$(srcdir)/../msglayer \
  -I$(srcdir)/..

noinst_LTLIBRARIES=libhbciaccjobs.la

noinst_HEADERS=\
  accountjob_l.h \
  accountjob_p.h \
  jobgetbalance_l.h \
  jobgetbalance_p.h \
  jobgettransactions_l.h \
  jobgettransactions_p.h \
  jobgetstandingorders_l.h \
  jobgetstandingorders_p.h \
  jobgetdatedxfers_l.h \
  jobgetdatedxfers_p.h \
  jobsingletransfer_l.h \
  jobsingletransfer_p.h \
  jobeutransfer_l.h \
  jobeutransfer_p.h \
  jobmultitransfer_l.h \
  jobmultitransfer_p.h

libhbciaccjobs_la_SOURCES=\
  accountjob.c \
  jobgetbalance.c \
  jobgettransactions.c \
  jobgetdatedxfers.c \
  jobgetstandingorders.c \
  jobsingletransfer.c \
  jobeutransfer.c \
  jobmultitransfer.c


sources:
	for f in $(libhbciaccjobs_la_SOURCES); do \
	  echo $(subdir)/$$f >>$(top_srcdir)/i18nsources; \
	done
	for d in $(SUBDIRS); do \
	  $(MAKE) -C $$d sources; \
	done

xmlfiles=\
  $(srcdir)/jobgetbalance.xml \
  $(srcdir)/jobgettransactions.xml \
  $(srcdir)/jobgetstandingorders.xml \
  $(srcdir)/jobsingletransfer.xml \
  $(srcdir)/jobsingledebitnote.xml \
  $(srcdir)/jobinternaltransfer.xml \
  $(srcdir)/jobeutransfer.xml \
  $(srcdir)/jobmultitransfer.xml \
  $(srcdir)/jobcreatesto.xml \
  $(srcdir)/jobmodifysto.xml \
  $(srcdir)/jobdeletesto.xml \
  $(srcdir)/jobgetdatedxfers.xml \
  $(srcdir)/jobcreatedatedxfer.xml \
  $(srcdir)/jobmodifydatedxfer.xml \
  $(srcdir)/jobdeletedatedxfer.xml

EXTRA_DIST=$(xmlfiles)

accountjobs.xml: $(xmlfiles)
	$(XMLMERGE) -v --compact -o $@ $(xmlfiles)

all-local: accountjobs.xml

CLEANFILES = accountjobs.xml
