
AM_CFLAGS=-DBUILDING_AQHBCI @visibility_cflags@

INCLUDES = -I$(aqbanking_symlinkdir) $(all_includes) \
  -I$(srcdir)/../joblayer \
  -I$(srcdir)/../msglayer \
  -I$(srcdir)/../banking \
  -I$(srcdir)/../applayer \
  -I$(srcdir)/..

noinst_LTLIBRARIES=libhbciadmjobs.la

noinst_HEADERS=\
  jobgetkeys_l.h jobgetkeys_p.h

libhbciadmjobs_la_SOURCES=\
  jobgetkeys.c


sources:
	for f in $(libhbciadmjobs_la_SOURCES); do \
	  echo $(subdir)/$$f >>$(top_srcdir)/i18nsources; \
	done
	for d in $(SUBDIRS); do \
	  $(MAKE) -C $$d sources; \
	done

xmlfiles=\
  $(srcdir)/jobgetkeys.xml

EXTRA_DIST=$(xmlfiles)

adminjobs.xml: $(xmlfiles)
	$(XMLMERGE) -v --compact -o $@ $(xmlfiles)

all-local: adminjobs.xml

CLEANFILES = adminjobs.xml


cppcheck:
	for f in $(libhbciadmjobs_la_SOURCES); do \
	  cppcheck --force $$f ; \
	done





