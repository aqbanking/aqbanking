
AM_CFLAGS=-DBUILDING_AQBANKING @visibility_cflags@

AM_CPPFLAGS = -I$(top_srcdir)/src/libs -I$(top_builddir)/src/libs $(gwenhywfar_includes) \
  -I$(srcdir)/../../ \
  -I$(srcdir)/../joblayer \
  -I$(srcdir)/../msglayer \
  -I$(srcdir)/../banking \
  -I$(srcdir)/../applayer \
  -I$(srcdir)/../tan \
  -I$(srcdir)/..

noinst_LTLIBRARIES=libhbciadmjobs.la

noinst_HEADERS=\
  jobgetkeys_l.h jobgetkeys_p.h \
  jobsendkeys_l.h jobsendkeys_p.h \
  jobchangekeys_l.h jobchangekeys_p.h \
  jobtan_l.h jobtan_p.h \
  jobgetsepainfo_l.h jobgetsepainfo_p.h \
  jobgetsysid_l.h jobgetsysid_p.h \
  jobgetbankinfo_l.h jobgetbankinfo_p.h \
  jobunblockpin_l.h \
  jobacknowledge_l.h


libhbciadmjobs_la_SOURCES=\
  jobgetkeys.c \
  jobsendkeys.c \
  jobchangekeys.c \
  jobtan.c \
  jobgetsepainfo.c \
  jobgetsysid.c \
  jobgetbankinfo.c \
  jobunblockpin.c \
  jobacknowledge.c



sources:
	for f in $(libhbciadmjobs_la_SOURCES); do \
	  echo $(subdir)/$$f >>$(top_srcdir)/i18nsources; \
	done
	for d in $(SUBDIRS); do \
	  $(MAKE) -C $$d sources; \
	done

xmlfiles=\
  $(srcdir)/jobgetkeys.xml \
  $(srcdir)/jobsendkeys.xml \
  $(srcdir)/jobchangekeys.xml \
  $(srcdir)/jobtan.xml \
  $(srcdir)/jobgetsepainfo.xml \
  $(srcdir)/jobgetsysid.xml \
  $(srcdir)/jobgetbankinfo.xml \
  $(srcdir)/jobunblockpin.xml \
  $(srcdir)/jobacknowledge.xml


EXTRA_DIST=$(xmlfiles)

adminjobs.xml: $(xmlfiles)
	$(XMLMERGE) -v --compact -o $@ $(xmlfiles)

all-local: adminjobs.xml

CLEANFILES = adminjobs.xml


cppcheck:
	for f in $(libhbciadmjobs_la_SOURCES); do \
	  cppcheck --force $$f ; \
	done





