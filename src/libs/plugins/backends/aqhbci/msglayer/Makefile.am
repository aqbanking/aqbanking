
AM_CFLAGS=-DBUILDING_AQBANKING @visibility_cflags@

AM_CPPFLAGS = -I$(top_srcdir)/src/libs -I$(top_builddir)/src/libs $(gwenhywfar_includes) \
  -I$(srcdir)/../.. \
  -I$(srcdir)/.. -I$(srcdir)/../banking \
  -I$(srcdir)/../applayer \
  -I$(srcdir)/../joblayer \
  -I$(srcdir)/../tan \
  -I$(aqbanking_symlinkdir) $(all_includes) \
  -I$(top_srcdir)/src/libs/aqbanking


EXTRA_DIST=\
  msgcrypt.c \
  msgcrypt_ddv.c \
  dialog_hbci.c \
  dialog_https.c

DEFS+=-DAH_XMLDATADIR=\"$(aqbanking_data_searchdir)/aqbanking/backends/aqhbci\"


noinst_LTLIBRARIES=libhbcimsg.la

noinst_HEADERS=\
 bpd_l.h \
 bpd_p.h \
 dialog_l.h \
 dialog_p.h \
 hbci_l.h \
 hbci_p.h \
 hbci-updates_l.h \
 hbci-updates_p.h \
 message_l.h \
 message_p.h \
 msgcrypt_rxh_common.h \
 msgcrypt_rxh_encrypt.h \
 msgcrypt_rxh_decrypt.h \
 msgcrypt_pintan.h \
 msgcrypt_pintan_verify.h \
 msgengine_l.h \
 msgengine_p.h \
 msgengine.h

#iheaderdir=@aqbanking_headerdir_am@/aqhbci
#iheader_HEADERS=

libhbcimsg_la_SOURCES=\
 bpd.c \
 dialog.c \
 hbci.c \
 hbci-updates.c \
 message.c \
 msgcrypt_rxh_common.c \
 msgcrypt_rxh_encrypt.c \
 msgcrypt_rxh_decrypt.c \
 msgcrypt_pintan.c \
 msgcrypt_pintan_verify.c \
 msgengine.c


sources:
	for f in $(libhbcimsg_la_SOURCES) $(EXTRA_DIST); do \
	  echo $(subdir)/$$f >>$(top_srcdir)/i18nsources; \
	done
	for d in $(SUBDIRS); do \
	  $(MAKE) -C $$d sources; \
	done



typedefs:
	for d in $(SUBDIRS); do \
	  $(MAKE) -C $$d typedefs; \
	done


typefiles: $(typefiles)
	typemaker -d AQHBCI_API $(typefiles)



cppcheck:
	for f in $(libhbcimsg_la_SOURCES); do \
	  cppcheck --force $$f ; \
	done
	for d in $(SUBDIRS); do \
	  $(MAKE) -C $$d cppcheck; \
	done


