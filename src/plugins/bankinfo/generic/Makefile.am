INCLUDES = $(all_includes)
EXTRA_DIST=dbb.conf hbci.conf atblz.conf README at.banks de.banks

bankinfoplugindir = $(aqbanking_plugindir)/bankinfo

DEFS += -DPLUGIN_DATADIR=\"$(aqbanking_pkgdatadir)/bankinfo\"

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

libbankinfo_la_LIBADD = -L$(top_builddir)/src/libs/aqbanking -laqbanking $(gwenhywfar_libs)
libbankinfo_la_LDFLAGS = $(STRIPALL)

at_files=data/at/blz.idx data/at/bic.idx data/at/namloc.idx data/at/banks.data
de_files=data/de/blz.idx data/de/bic.idx data/de/namloc.idx data/de/banks.data

de.banks.bz2:
	./mkdeinfo build data/blz.csv data/fints_institute.csv de.banks && \
	bzip2 -9 de.banks

de.banks: de.banks.bz2
	bunzip2 -f -k de.banks.bz2

$(de_files): de.banks
	test -d data/de || \
	  mkdir -p data/de
	./mkdeinfo install de.banks data/de

de-data: $(de_files)

install-de: de-data
	test -d $(DESTDIR)/@aqbanking_pkgdatadir@/bankinfo/de || \
	  mkdir -p $(DESTDIR)/@aqbanking_pkgdatadir@/bankinfo/de
	cp $(de_files) $(DESTDIR)/@aqbanking_pkgdatadir@/bankinfo/de/

at.banks.bz2:
	./mkdeinfo build-at data/blz-at.csv at.banks && \
	bzip2 -9 at.banks

at.banks: at.banks.bz2
	bunzip2 -f -k at.banks.bz2

$(at_files): at.banks
	test -d data/at || \
	  mkdir -p data/at
	./mkdeinfo install at.banks data/at

at-data: $(at_files)

install-at: at-data
	test -d $(DESTDIR)/@aqbanking_pkgdatadir@/bankinfo/at || \
	  mkdir -p $(DESTDIR)/@aqbanking_pkgdatadir@/bankinfo/at
	cp $(at_files) $(DESTDIR)/@aqbanking_pkgdatadir@/bankinfo/at/

bankdata: at-data de-data

install-bankdata: install-at install-de

install-data-local: install-bankdata

dist-local: at.banks de.banks

noinst_PROGRAMS=mkdeinfo
mkdeinfo_SOURCES=mkdeinfo.c
mkdeinfo_LDADD = -L$(top_builddir)/src/libs/aqbanking -laqbanking $(gwenhywfar_libs)

sources:
	for f in $(de_la_SOURCES); do \
	  echo $(subdir)/$$f >>$(top_srcdir)/i18nsources; \
	done
