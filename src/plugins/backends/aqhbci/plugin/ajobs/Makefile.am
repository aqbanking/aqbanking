
AM_CFLAGS=-DBUILDING_AQHBCI @visibility_cflags@

INCLUDES = -I$(aqbanking_symlinkdir) $(all_includes) \
  -I$(srcdir)/../joblayer \
  -I$(srcdir)/../msglayer \
  -I$(srcdir)/../banking \
  -I$(srcdir)/../applayer \
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
  jobmultitransfer_p.h \
  jobforeignxferwh_l.h \
  jobforeignxferwh_p.h \
  jobloadcellphone_l.h \
  jobloadcellphone_p.h \
  jobsepadebitdatedsinglecreate_l.h \
  jobsepadebitdatedsinglecreate_p.h \
  jobsepadebitdatedmulticreate_l.h \
  jobsepadebitdatedmulticreate_p.h \
  jobsepaxfermulti_l.h \
  jobsepaxfermulti_p.h \
  jobsepaxfersingle_l.h \
  jobsepaxfersingle_p.h \
  jobsepadebitsingle_l.h \
  jobsepadebitsingle_p.h \
  jobsepacor1datedsinglecreate_l.h \
  jobsepacor1datedsinglecreate_p.h \
  jobtransferbase_l.h jobtransferbase_p.h \
  jobsepastandingordercreate_l.h jobsepastandingordercreate_p.h \
  jobsepastandingorderget_l.h jobsepastandingorderget_p.h

libhbciaccjobs_la_SOURCES=\
  accountjob.c \
  jobgetbalance.c \
  jobgettransactions.c \
  jobgetdatedxfers.c \
  jobgetstandingorders.c \
  jobsingletransfer.c \
  jobeutransfer.c \
  jobmultitransfer.c \
  jobforeignxferwh.c \
  jobloadcellphone.c \
  jobsepadebitdatedsinglecreate.c \
  jobsepadebitdatedmulticreate.c \
  jobsepaxfermulti.c \
  jobsepaxfersingle.c \
  jobsepadebitsingle.c \
  jobsepacor1datedsinglecreate.c \
  jobtransferbase.c \
  jobsepastandingordercreate.c \
  jobsepastandingorderget.c



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
  $(srcdir)/jobforeignxferwh.xml \
  $(srcdir)/jobcreatesto.xml \
  $(srcdir)/jobmodifysto.xml \
  $(srcdir)/jobdeletesto.xml \
  $(srcdir)/jobgetdatedxfers.xml \
  $(srcdir)/jobcreatedatedxfer.xml \
  $(srcdir)/jobmodifydatedxfer.xml \
  $(srcdir)/jobdeletedatedxfer.xml \
  $(srcdir)/jobloadcellphone.xml \
  $(srcdir)/jobsepadebitdatedsinglecreate.xml \
  $(srcdir)/jobsepadebitdatedmulticreate.xml \
  $(srcdir)/jobsepaxfermulti.xml \
  $(srcdir)/jobsepaxfersingle.xml \
  $(srcdir)/jobsepadebitsingle.xml \
  $(srcdir)/jobsepacor1datedsinglecreate.xml \
  $(srcdir)/jobsepastandingordercreate.xml \
  $(srcdir)/jobsepastandingorderget.xml


EXTRA_DIST=$(xmlfiles)

accountjobs.xml: $(xmlfiles)
	$(XMLMERGE) -v --compact -o $@ $(xmlfiles)

all-local: accountjobs.xml

CLEANFILES = accountjobs.xml
