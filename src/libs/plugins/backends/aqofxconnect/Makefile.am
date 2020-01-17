SUBDIRS=libofxhome v2 control dialogs

AM_CPPFLAGS = -I$(top_srcdir)/src/libs -I$(top_builddir)/src/libs $(gwenhywfar_includes) -I$(srcdir)/..

AM_CFLAGS=-DBUILDING_AQBANKING @visibility_cflags@

localedir=$(datadir)/locale
DEFS += -DLOCALEDIR=\"$(localedir)\"

plugindir = $(aqbanking_plugindir)/providers
plugin_DATA = aqofxconnect.xml


noinst_HEADERS=\
  account_p.h \
  aqofxconnect_l.h \
  provider_p.h \
  user_p.h \
  account.h \
  aqofxconnect.h \
  provider.h \
  user.h


#iheaderdir=@aqbanking_headerdir_am@/aqofxconnect
#iheader_HEADERS=


# static plugin library
noinst_LTLIBRARIES=libaqofxconnect.la 
libaqofxconnect_la_SOURCES= \
  account.c \
  user.c \
  provider.c

libaqofxconnect_la_LIBADD= \
  libofxhome/libofxhome.la \
  v2/libofxdcv2.la \
  control/libofxdccontrol.la \
  dialogs/libofxdcdialogs.la



EXTRA_DIST= $(dlgdata_DATA) $(dlgimage_DATA) \
  aqofxconnect.xml.in \
  provider_accspec.c \
  provider_sendcmd.c \
  provider_update.c


sources:
	for f in $(libaqofxconnect_la_SOURCES) $(aqofxconnect_la_SOURCES); do \
	  echo $(subdir)/$$f >>$(top_srcdir)/i18nsources; \
	done
	for f in $(dlgdata_DATA); do \
	  echo $(subdir)/$$f >>$(top_srcdir)/dlgsources; \
	done
	for f in $(plugin_DATA); do \
	  echo $(subdir)/$$f >>$(top_srcdir)/pdsources; \
	done
	for d in $(SUBDIRS); do \
	  $(MAKE) -C $$d sources; \
	done

built_sources:
	list='$(SUBDIRS)'; for d in $$list; do \
	  $(MAKE) -C $$d built_sources; \
	done ;


cppcheck:
	for f in $(libaqofxconnect_la_SOURCES); do \
	  cppcheck --force $$f ; \
	done
	for d in $(SUBDIRS); do \
	  $(MAKE) -C $$d cppcheck; \
	done


typefiles:
	for d in $(SUBDIRS); do \
	  $(MAKE) -C $$d typefiles; \
	done

typedefs:
	for d in $(SUBDIRS); do \
	  $(MAKE) -C $$d typedefs; \
	done

