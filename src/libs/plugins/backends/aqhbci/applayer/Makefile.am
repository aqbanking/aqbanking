SUBDIRS=xml

AM_CFLAGS=-DBUILDING_AQBANKING @visibility_cflags@

AM_CPPFLAGS = -I$(top_srcdir)/src/libs -I$(top_builddir)/src/libs $(gwenhywfar_includes) \
  -I$(srcdir)/../../ \
  -I$(srcdir)/../ajobs \
  -I$(srcdir)/../admjobs \
  -I$(srcdir)/../banking \
  -I$(srcdir)/../joblayer \
  -I$(srcdir)/../msglayer \
  -I$(srcdir)/../tan \
  -I$(srcdir)/..

EXTRA_DIST=\
  README


noinst_LTLIBRARIES=libhbciapp.la

noinst_HEADERS=\
  adminjobs_l.h adminjobs_p.h\
  outbox_l.h outbox_p.h \
  hhd_l.h hhd_p.h \
  cbox.h \
  cbox_p.h \
  cbox_itan.h \
  cbox_itan1.h \
  cbox_itan2.h \
  cbox_send.h \
  cbox_recv.h \
  cbox_psd2.h \
  cbox_hbci.h \
  cbox_dialog.h \
  cbox_prepare.h \
  cbox_queue.h


libhbciapp_la_SOURCES=\
  adminjobs.c \
  hhd.c \
  outbox.c \
  cbox.c \
  cbox_recv.c \
  cbox_send.c \
  cbox_psd2.c \
  cbox_hbci.c \
  cbox_dialog.c \
  cbox_itan.c \
  cbox_itan1.c \
  cbox_itan2.c \
  cbox_prepare.c \
  cbox_queue.c



sources:
	for f in $(libhbciapp_la_SOURCES); do \
	  echo $(subdir)/$$f >>$(top_srcdir)/i18nsources; \
	done
	for d in $(SUBDIRS); do \
	  $(MAKE) -C $$d sources; \
	done

CLEANFILES = $(xmldata_DATA) tmp.xml


typefiles:
	for d in $(SUBDIRS); do \
	  $(MAKE) -C $$d typefiles; \
	done

typedefs:
	for d in $(SUBDIRS); do \
	  $(MAKE) -C $$d typedefs; \
	done




cppcheck:
	for f in $(libhbciapp_la_SOURCES); do \
	  cppcheck --force $$f ; \
	done


