

INCLUDES=-I$(srcdir)/.. -I$(top_builddir) $(gwenhywfar_includes) -I$(aqbanking_symlinkdir) $(AQEBICS_CFLAGS)

DEFS += -DLOCALEDIR=\"$(localedir)\"
AM_CFLAGS=-DBUILDING_AQEBICS @visibility_cflags@

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
  p_tools.c \
  r_hia.c \
  r_hia_h002.c \
  r_hia_h003.c \
  r_hia_h004.c \
  r_hpb.c \
  r_hpb_h002.c \
  r_hpb_h003.c \
  r_hpb_h004.c \
  r_hpd.c \
  r_ini.c \
  r_ini_h002.c \
  r_ini_h003.c \
  r_ini_h004.c \
  r_download.c \
  r_download_h002.c \
  r_download_h003.c \
  r_sta.c \
  r_upload.c \
  r_upload_h002.c \
  r_upload_h003.c \
  r_pub.c \
  r_pub_h002.c \
  r_pub_h003.c


EXTRA_DIST=$(typefiles) $(extra_sources)


noinst_HEADERS=\
 account_l.h \
 account_p.h \
 dialog_l.h \
 dialog_p.h \
 r_hkd_htd_l.h \
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
 r_hkd_htd.c \
 provider.c


#types: $(typefiles)
#	typemaker -d static $(typefiles)

types: $(typefiles)
	typemaker $(typefiles)

sources:
	for d in $(SUBDIRS); do \
	  $(MAKE) -C $$d sources; \
	done
	for f in $(libclient_la_SOURCES) $(EXTRA_SOURCES); do \
	  echo $(subdir)/$$f >>$(top_srcdir)/i18nsources; \
	done


cppcheck:
	for f in $(libclient_la_SOURCES); do \
	  cppcheck --force $$f ; \
	done
	for d in $(SUBDIRS); do \
	  $(MAKE) -C $$d cppcheck; \
	done

