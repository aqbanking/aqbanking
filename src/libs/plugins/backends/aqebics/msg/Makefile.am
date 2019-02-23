

INCLUDES=-I.. -I$(top_builddir) -I$(srcdir)/../client  \
  $(AQEBICS_CFLAGS) $(gwenhywfar_includes) -I$(aqbanking_symlinkdir)

AM_CFLAGS=-DBUILDING_AQEBICS @visibility_cflags@

noinst_LTLIBRARIES=libmsg.la

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


sources:
	for d in $(SUBDIRS); do \
	  $(MAKE) -C $$d sources; \
	done
	for f in $(libmsg_la_SOURCES); do \
	  echo $(subdir)/$$f >>$(top_srcdir)/i18nsources; \
	done


cppcheck:
	for f in $(testlib_SOURCES); do \
	  cppcheck --force $$f ; \
	done
	for d in $(SUBDIRS); do \
	  $(MAKE) -C $$d cppcheck; \
	done


