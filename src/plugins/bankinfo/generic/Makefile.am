INCLUDES = $(all_includes)
EXTRA_DIST=dbb.conf hbci.conf atblz.conf README at.banks de.banks

bankinfoplugindir=@aqbanking_plugindir@/bankinfo

DEFS=@DEFS@ -DPLUGIN_DATADIR=\"@aqbanking_pkgdatadir@/bankinfo\"

noinst_LTLIBRARIES=libbankinfo.la
#bankinfoplugin_DATA=de.xml

if IS_WINDOWS
  CHMOD=echo
else
  CHMOD=/bin/chmod
endif


noinst_HEADERS=\
 generic_p.h \
 generic_l.h

libbankinfo_la_SOURCES=\
 generic.c

libbankinfo_la_LIBADD=@gwenhywfar_libs@ -L../../../libs/aqbanking -laqbanking
libbankinfo_la_LDFLAGS=@STRIPALL@

de.banks.bz2:
	./mkdeinfo build data/blz.csv data/fints_institute.csv de.banks && \
	bzip2 -9 de.banks

de.banks: de.banks.bz2
	bunzip2 -f -k de.banks.bz2

install-de:
	./mkdeinfo install de.banks $(DESTDIR)/@aqbanking_pkgdatadir@/bankinfo/de

at.banks.bz2:
	./mkdeinfo build-at data/blz-at.csv at.banks && \
	bzip2 -9 at.banks

at.banks: at.banks.bz2
	bunzip2 -f -k at.banks.bz2

install-at:
	./mkdeinfo install at.banks $(DESTDIR)/@aqbanking_pkgdatadir@/bankinfo/at

install-bankdata: install-de install-at

install-data-local: install-bankdata

dist-local: at.banks de.banks

noinst_PROGRAMS=mkdeinfo
mkdeinfo_SOURCES=mkdeinfo.c
mkdeinfo_LDADD=@gwenhywfar_libs@ -L../../../libs/aqbanking -laqbanking

sources:
	for f in $(de_la_SOURCES); do \
	  echo $(subdir)/$$f >>$(top_srcdir)/i18nsources; \
	done
