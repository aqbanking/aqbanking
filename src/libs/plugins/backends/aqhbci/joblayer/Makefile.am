
AM_CFLAGS=-DBUILDING_AQBANKING @visibility_cflags@

AM_CPPFLAGS = -I$(top_srcdir)/src/libs -I$(top_builddir)/src/libs $(gwenhywfar_includes) \
  -I$(srcdir)/../.. \
  -I$(srcdir)/../tan \
  -I$(srcdir)/../msglayer -I$(srcdir)/..

noinst_LTLIBRARIES=libhbcijob.la

noinst_HEADERS=\
  job_l.h \
  job_p.h \
  job_commit.h \
  job_commit_key.h \
  job_crypt.h \
  job_swift.h \
  jobqueue_l.h \
  jobqueue_p.h \
  jobqueue_dispatch.h \
  jobqueue_tomsg.h \
  jobqueue_addjob.h \
  jobqueue_bpd.h \
  jobqueue_account.h \
  result_l.h \
  result_p.h

#iheaderdir=@aqbanking_headerdir_am@/aqhbci
#iheader_HEADERS=

libhbcijob_la_SOURCES=\
  job.c \
  job_commit.c \
  job_commit_key.c \
  job_crypt.c \
  job_swift.c \
  jobqueue.c \
  jobqueue_dispatch.c \
  jobqueue_tomsg.c \
  jobqueue_addjob.c \
  jobqueue_bpd.c \
  jobqueue_account.c \
  result.c


EXTRA_DIST=\
  job_virtual.c \
  job_new.c



typedefs:
	for d in $(SUBDIRS); do \
	  $(MAKE) -C $$d typedefs; \
	done


typefiles: $(typefiles)
	$(gwenhywfar_bindir)/typemaker -d AQHBCI_API $(typefiles)

sources:
	for f in $(libhbcijob_la_SOURCES); do \
	  echo $(subdir)/$$f >>$(top_srcdir)/i18nsources; \
	done
	for d in $(SUBDIRS); do \
	  $(MAKE) -C $$d sources; \
	done


cppcheck:
	for f in $(libhbcijob_la_SOURCES); do \
	  cppcheck --force $$f ; \
	done
	for d in $(SUBDIRS); do \
	  $(MAKE) -C $$d cppcheck; \
	done


