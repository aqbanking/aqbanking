
AM_CFLAGS=-DBUILDING_AQBANKING @visibility_cflags@

AM_CPPFLAGS = -I$(top_srcdir)/src/libs -I$(top_builddir)/src/libs $(gwenhywfar_includes) -I$(srcdir)/../..


localedir = $(datadir)/locale
DEFS += -DLOCALEDIR=\"$(localedir)\"
noinst_LTLIBRARIES=libofxdccontrol.la

noinst_HEADERS=\
 control.h \
 adduser.h \
 getaccounts.h \
 listusers.h



#iheaderdir=@aqbanking_headerdir_am@/aqpaypal
#iheader_HEADERS=

libofxdccontrol_la_SOURCES=\
 control.c \
 adduser.c \
 getaccounts.c \
 listusers.c



#EXTRA_DIST=



sources:
	for f in $(libofxdccontrol_la_SOURCES); do \
	  echo $(subdir)/$$f >>$(top_srcdir)/i18nsources; \
	done
	for d in $(SUBDIRS); do \
	  $(MAKE) -C $$d sources; \
	done


cppcheck:
	for f in $(libofxdccontrol_la_SOURCES); do \
	  cppcheck --force $$f ; \
	done
	for d in $(SUBDIRS); do \
	  $(MAKE) -C $$d cppcheck; \
	done


built_sources:
	list='$(SUBDIRS)'; for d in $$list; do \
	  $(MAKE) -C $$d built_sources; \
	done ;


typefiles:

typedefs:


