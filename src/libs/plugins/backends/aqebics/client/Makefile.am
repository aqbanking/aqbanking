

AM_CPPFLAGS = -I$(top_srcdir)/src/libs -I$(top_builddir)/src/libs $(gwenhywfar_includes) -I$(srcdir)/../.. $(AQEBICS_CFLAGS)

DEFS += -DLOCALEDIR=\"$(localedir)\"
AM_CFLAGS=-DBUILDING_AQBANKING @visibility_cflags@

#typefiles=context.xml

noinst_LTLIBRARIES=libclient.la
noinst_PROGRAMS=testlib

extra_sources=\
  provider_sendcmd.c \
  provider_accspec.c \
  p_encipher.c \
  p_encipher_e001.c \
  p_encipher_e002.c \
  p_decipher.c \
  p_eu.c \
  p_eu_a004.c \
  p_eu_a005.c \
  p_sign.c \
  p_sign_x001.c \
  p_sign_x002.c \
  p_tools.c


EXTRA_DIST=$(typefiles) $(extra_sources)


noinst_HEADERS=\
 account_l.h \
 account_p.h \
 dialog_l.h \
 dialog_p.h \
 provider_l.h \
 provider_p.h \
 user_l.h \
 user_p.h \
 account.h \
 provider.h \
 user.h

#iheaderdir=${includedir}/aqebics/client
#iheader_HEADERS=


libclient_la_SOURCES=\
 account.c \
 user.c \
 dialog.c \
 provider.c


#types: $(typefiles)
#	typemaker -d static $(typefiles)

types: $(typefiles)
	typemaker $(typefiles)

sources:
	for d in $(SUBDIRS); do \
	  $(MAKE) -C $$d sources; \
	done
	for f in $(libclient_la_SOURCES) $(extra_sources); do \
	  echo $(subdir)/$$f >>$(top_srcdir)/i18nsources; \
	done


cppcheck:
	for f in $(libclient_la_SOURCES); do \
	  cppcheck --force $$f ; \
	done
	for d in $(SUBDIRS); do \
	  $(MAKE) -C $$d cppcheck; \
	done

