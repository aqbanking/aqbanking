
AM_CFLAGS=-DBUILDING_AQBANKING @visibility_cflags@

AM_CPPFLAGS = -I$(top_srcdir)/src/libs -I$(top_builddir)/src/libs $(gwenhywfar_includes) -I$(srcdir)/../..


localedir = $(datadir)/locale
DEFS += -DLOCALEDIR=\"$(localedir)\"
noinst_LTLIBRARIES=libpaypalcontrol.la

noinst_HEADERS=\
 globals_l.h \
 control_l.h


#iheaderdir=@aqbanking_headerdir_am@/aqpaypal
#iheader_HEADERS=

libpaypalcontrol_la_SOURCES=\
 main.c \
 adduser.c \
 delaccount.c \
 listaccounts.c \
 listusers.c \
 deluser.c \
 setsecrets.c



#EXTRA_DIST=



sources:
	for f in $(libpaypalcontrol_la_SOURCES); do \
	  echo $(subdir)/$$f >>$(top_srcdir)/i18nsources; \
	done
	for d in $(SUBDIRS); do \
	  $(MAKE) -C $$d sources; \
	done


cppcheck:
	for f in $(libpaypalcontrol_la_SOURCES); do \
	  cppcheck --force $$f ; \
	done
	for d in $(SUBDIRS); do \
	  $(MAKE) -C $$d cppcheck; \
	done


