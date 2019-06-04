SUBDIRS=xml

AM_CFLAGS=-DBUILDING_AQHBCI @visibility_cflags@

INCLUDES = -I$(top_srcdir)/src/libs -I$(top_builddir)/src/libs $(gwenhywfar_includes) \
  -I$(srcdir)/../../ \
  -I$(srcdir)/../ajobs \
  -I$(srcdir)/../admjobs \
  -I$(srcdir)/../banking \
  -I$(srcdir)/../joblayer \
  -I$(srcdir)/../msglayer \
  -I$(srcdir)/..

EXTRA_DIST=\
  README \
  itan.c \
  itan1.c \
  itan2.c

noinst_LTLIBRARIES=libhbciapp.la

noinst_HEADERS=\
  adminjobs_l.h adminjobs_p.h\
  outbox_l.h outbox_p.h \
  hhd_l.h hhd_p.h \
  tanmechanism.h tanmechanism_p.h \
  tan_chiptan_opt.h \
  tan_text.h


libhbciapp_la_SOURCES=\
  adminjobs.c \
  outbox.c \
  hhd.c \
  tanmechanism.c \
  tan_chiptan_opt.c \
  tan_text.c



sources:
	for f in $(libhbciapp_la_SOURCES); do \
	  echo $(subdir)/$$f >>$(top_srcdir)/i18nsources; \
	done
	for d in $(SUBDIRS); do \
	  $(MAKE) -C $$d sources; \
	done

CLEANFILES = $(xmldata_DATA) tmp.xml



cppcheck:
	for f in $(libhbciapp_la_SOURCES); do \
	  cppcheck --force $$f ; \
	done


