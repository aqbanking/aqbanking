
AM_CFLAGS=-DBUILDING_AQPAYPAL @visibility_cflags@

INCLUDES = -I$(aqbanking_symlinkdir) $(gwenhywfar_includes) \
  -I$(srcdir)/../msglayer -I$(srcdir)/../joblayer \
  -I$(srcdir)/../applayer -I$(srcdir)/../ajobs  -I$(srcdir)/.. \
  -I$(srcdir)/../dialogs


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
 listaccounts.c \
 listusers.c \
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


