
AM_CFLAGS=-DBUILDING_AQHBCI @visibility_cflags@

INCLUDES = -I$(aqbanking_symlinkdir) $(all_includes) \
  -I$(srcdir)/../msglayer -I$(srcdir)/..

typefiles=tanmethod.xml

EXTRA_DIST=$(typefiles)

noinst_LTLIBRARIES=libhbcijob.la

noinst_HEADERS=\
  job_l.h \
  job_p.h \
  jobqueue_l.h \
  jobqueue_p.h \
  result_l.h \
  result_p.h \
  tanmethod_l.h \
  tanmethod_p.h

iheaderdir=@aqbanking_headerdir_am@/aqhbci
iheader_HEADERS=\
  tanmethod.h

libhbcijob_la_SOURCES=\
  job.c \
  jobqueue.c \
  result.c \
  tanmethod.c


types: $(typefiles)
	$(gwenhywfar_bindir)/typemaker -d AQHBCI_API $(typefiles)

sources:
	for f in $(libhbcijob_la_SOURCES); do \
	  echo $(subdir)/$$f >>$(top_srcdir)/i18nsources; \
	done
	for d in $(SUBDIRS); do \
	  $(MAKE) -C $$d sources; \
	done

