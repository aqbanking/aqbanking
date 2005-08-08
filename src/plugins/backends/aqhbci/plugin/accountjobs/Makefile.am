
INCLUDES = -I$(top_builddir)/headers $(all_includes) \
  -I$(srcdir)/../joblayer \
  -I$(srcdir)/../msglayer \
  -I$(srcdir)/..

noinst_LTLIBRARIES=libhbciaccjobs.la

noinst_HEADERS=\
  accountjob_l.h \
  accountjob_p.h \
  jobgetbalance_p.h \
  jobgettransactions_p.h \
  jobgetstandingorders_p.h \
  jobgetdatedxfers_p.h \
  jobsingletransfer_p.h \
  jobeutransfer_p.h \
  jobmultitransfer_l.h \
  jobmultitransfer_p.h

iheaderdir=${includedir}/aqhbci
iheader_HEADERS=\
  accountjob.h \
  jobgetbalance.h \
  jobgettransactions.h \
  jobgetstandingorders.h \
  jobgetdatedxfers.h \
  jobsingletransfer.h \
  jobeutransfer.h \
  jobmultitransfer.h

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
	  make -C $$d sources; \
	done

xmlfiles=\
  $(srcdir)/jobgetbalance.xml \
  $(srcdir)/jobgettransactions.xml \
  $(srcdir)/jobgetstandingorders.xml \
  $(srcdir)/jobsingletransfer.xml \
  $(srcdir)/jobsingledebitnote.xml \
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
