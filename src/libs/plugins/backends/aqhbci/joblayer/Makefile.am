
AM_CFLAGS=-DBUILDING_AQHBCI @visibility_cflags@

INCLUDES = -I$(top_srcdir)/src/libs -I$(top_builddir)/src/libs $(gwenhywfar_includes) \
  -I$(srcdir)/../.. \
  -I$(srcdir)/../msglayer -I$(srcdir)/..

typefiles=tanmethod.xml

noinst_LTLIBRARIES=libhbcijob.la

noinst_HEADERS=\
  job_l.h \
  job_p.h \
  jobqueue_l.h \
  jobqueue_p.h \
  result_l.h \
  result_p.h \
  tanmethod_l.h \
  tanmethod_p.h \
  tanmethod.h

#iheaderdir=@aqbanking_headerdir_am@/aqhbci
#iheader_HEADERS=

libhbcijob_la_SOURCES=\
  job.c \
  jobqueue.c \
  result.c \
  tanmethod.c


EXTRA_DIST=$(typefiles) \
  job_commit.c \
  job_commit_account.c \
  job_virtual.c




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


