

INCLUDES=-I.. -I$(top_builddir) \
  $(AQEBICS_CFLAGS) $(gwenhywfar_includes) -I$(aqbanking_symlinkdir)

AM_CFLAGS=-DBUILDING_AQEBICS @visibility_cflags@

noinst_LTLIBRARIES=libmsg.la
noinst_PROGRAMS=testlib

libmsg_la_SOURCES=\
 keys.c \
 msg.c \
 xml.c \
 zip.c \
 eu.c

noinst_HEADERS=\
 eu.h \
 eu_p.h \
 keys.h \
 msg.h \
 msg_p.h \
 xml.h \
 xml_p.h \
 zip.h


testlib_SOURCES=testlib.c
testlib_LDADD=libmsg.la $(AQEBICS_LIBS) $(gwenhywfar_libs) -lz

check-local: testlib$(EXEEXT)
	./testlib check1
	./testlib check2
	./testlib check3

sources:
	for d in $(SUBDIRS); do \
	  $(MAKE) -C $$d sources; \
	done
	for f in $(libmsg_la_SOURCES); do \
	  echo $(subdir)/$$f >>$(top_srcdir)/i18nsources; \
	done
