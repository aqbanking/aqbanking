
AM_CFLAGS=-DBUILDING_AQBANKING @visibility_cflags@

AM_CPPFLAGS = -I$(top_srcdir)/src/libs -I$(top_builddir)/src/libs $(gwenhywfar_includes) \
  -I$(srcdir)/../.. \
  -I$(srcdir)/../tan \
  -I$(srcdir)/../msglayer -I$(srcdir)/..

noinst_LTLIBRARIES=libhbcijob.la

noinst_HEADERS=\
  job_l.h \
  job_p.h \
  jobqueue_l.h \
  jobqueue_p.h \
  result_l.h \
  result_p.h

#iheaderdir=@aqbanking_headerdir_am@/aqhbci
#iheader_HEADERS=

libhbcijob_la_SOURCES=\
  job.c \
  jobqueue.c \
  result.c


EXTRA_DIST=\
  job_commit.c \
  job_commit_account.c \
  job_virtual.c \
  job_new.c




types: $(typefiles)
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


