SUBDIRS=xml

AM_CFLAGS=-DBUILDING_AQHBCI @visibility_cflags@

INCLUDES = -I$(top_builddir)/headers $(all_includes) \
  -I$(srcdir)/../accountjobs \
  -I$(srcdir)/../joblayer -I$(srcdir)/../msglayer -I$(srcdir)/..

EXTRA_DIST=README

noinst_LTLIBRARIES=libhbciapp.la

noinst_HEADERS=\
  adminjobs_l.h \
  adminjobs_p.h\
  outbox_l.h \
  outbox_p.h

libhbciapp_la_SOURCES=\
  adminjobs.c \
  outbox.c

sources:
	for f in $(libhbciapp_la_SOURCES); do \
	  echo $(subdir)/$$f >>$(top_srcdir)/i18nsources; \
	done
	for d in $(SUBDIRS); do \
	  make -C $$d sources; \
	done

CLEANFILES = $(xmldata_DATA) tmp.xml
