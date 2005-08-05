INCLUDES = -I$(top_builddir)/headers $(all_includes) \
  -I$(srcdir)/../msglayer -I$(srcdir)/..

noinst_LTLIBRARIES=libhbcijob.la

noinst_HEADERS=\
  job_p.h \
  job_l.h \
  jobqueue_p.h \
  jobqueue_l.h \
  result_p.h

iheaderdir=${includedir}/aqhbci
iheader_HEADERS=\
  job.h \
  jobqueue.h \
  result.h

libhbcijob_la_SOURCES=\
  job.c \
  jobqueue.c \
  result.c

sources:
	for f in $(libhbcijob_la_SOURCES); do \
	  echo $(subdir)/$$f >>$(top_srcdir)/i18nsources; \
	done
	for d in $(SUBDIRS); do \
	  make -C $$d sources; \
	done

