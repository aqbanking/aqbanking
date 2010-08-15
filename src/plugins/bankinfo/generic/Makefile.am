INCLUDES = -I$(aqbanking_symlinkdir) $(gwenhywfar_includes) \
           -I$(top_srcdir)/src/libs/aqbanking

EXTRA_DIST=\
  dbb.conf hbci.conf atblz.conf fedachdir.conf bcbankenstamm.conf \
  kidaten.conf README at.tar.bz2 ch.tar.bz2 de.tar.bz2 us.tar.bz2 ca.tar.bz2

bankinfoplugindir = $(aqbanking_plugindir)/bankinfo
bankinfodatadir = $(aqbanking_pkgdatadir)/bankinfo

#DEFS += -DPLUGIN_DATADIR=\"$(bankinfodatadir)\"

noinst_LTLIBRARIES=libbankinfo_generic.la
noinst_HEADERS=\
 generic_p.h \
 generic_l.h

libbankinfo_generic_la_SOURCES=generic.c

libbankinfo_generic_la_LIBADD = $(aqbanking_internal_libs) $(gwenhywfar_libs)
libbankinfo_generic_la_LDFLAGS = $(STRIPALL)

at_files=at/blz.idx at/bic.idx at/namloc.idx at/banks.data
ch_files=ch/blz.idx ch/bic.idx ch/namloc.idx ch/banks.data
de_files=de/blz.idx de/bic.idx de/namloc.idx de/banks.data
us_files=us/blz.idx us/bic.idx us/namloc.idx us/banks.data
ca_files=ca/blz.idx ca/bic.idx ca/namloc.idx ca/banks.data

atbankdatadir = $(bankinfodatadir)/at
atbankdata_DATA = $(at_files)

debankdatadir = $(bankinfodatadir)/de
debankdata_DATA = $(de_files)

chbankdatadir = $(bankinfodatadir)/ch
chbankdata_DATA = $(ch_files)

usbankdatadir = $(bankinfodatadir)/us
usbankdata_DATA = $(us_files)

cabankdatadir = $(bankinfodatadir)/ca
cabankdata_DATA = $(ca_files)

at.tar.bz2:
	./mkdeinfo build-at data/at/kidaten.csv.utf8 tmp.banks
	test -d at || mkdir -p at
	if test -f data/at/aux.conf; then \
	  ./mkdeinfo update tmp.banks data/at/aux.conf tmp.banks; \
	fi
	./mkdeinfo install tmp.banks at
	tar cf at.tar at && bzip2 -9 at.tar
	rm -rf tmp.banks

$(at_files): at_files_tmp

at_files_tmp: at.tar.bz2
	test -d at || mkdir -p at
	bunzip2 -k -c $< | tar xmf -
	touch $@

at-daten: data/at/kidaten.csv
	$(SED) s/\"//g <data/at/kidaten.csv >data/at/kidaten.csv.out
	iconv -f iso8859-15 -t utf-8 data/at/kidaten.csv.out -o data/at/kidaten.csv.utf8

ch.tar.bz2:
	./mkdeinfo build-ch data/ch/bcbankenstamm.csv tmp.banks
	test -d ch || mkdir -p ch
	if test -f data/ch/aux.conf; then \
	  ./mkdeinfo update tmp.banks data/ch/aux.conf tmp.banks; \
	fi
	./mkdeinfo install tmp.banks ch
	tar cf ch.tar ch && bzip2 -9 ch.tar
	rm -rf tmp.banks

$(ch_files): ch_files_tmp

ch_files_tmp: ch.tar.bz2
	test -d ch || mkdir -p ch
	bunzip2 -k -c $< | tar xmf -
	touch $@

de.tar.bz2:
	./mkdeinfo build data/de/blz.csv data/de/fints_institute.csv tmp.banks
	test -d de || mkdir -p de
	if test -f data/de/aux.conf; then \
	  ./mkdeinfo update tmp.banks data/de/aux.conf tmp.banks; \
	fi
	./mkdeinfo install tmp.banks de
	tar cf de.tar de && bzip2 -9 de.tar
	rm -rf tmp.banks

$(de_files): de_files_tmp

de_files_tmp: de.tar.bz2
	test -d de || mkdir -p de
	bunzip2 -k -c $< | tar xmf -
	touch $@

us.tar.bz2:
	./mkdeinfo build-am data/us/fidata.UTF-8 us data/us/FedACHdir.txt tmp.banks
	test -d us || mkdir -p us
	if test -f data/us/aux.conf; then \
	  ./mkdeinfo update tmp.banks data/us/aux.conf tmp.banks; \
	fi
	./mkdeinfo install tmp.banks us
	tar cf us.tar us && bzip2 -9 us.tar
	rm -rf tmp.banks

$(us_files): us_files_tmp

us_files_tmp: us.tar.bz2
	test -d us || mkdir -p us
	bunzip2 -k -c $< | tar xmf -
	touch $@


ca.tar.bz2:
	./mkdeinfo build-am data/us/fidata.UTF-8 ca data/us/FedACHdir.txt tmp.banks
	test -d ca || mkdir -p ca
	if test -f data/ca/aux.conf; then \
	  ./mkdeinfo update tmp.banks data/ca/aux.conf tmp.banks; \
	fi
	./mkdeinfo install tmp.banks ca
	tar cf ca.tar ca && bzip2 -9 ca.tar
	rm -rf tmp.banks

$(ca_files): ca_files_tmp

ca_files_tmp: ca.tar.bz2
	test -d ca || mkdir -p ca
	bunzip2 -k -c $< | tar xmf -
	touch $@


.INTERMEDIATE: at_files_tmp ch_files_tmp de_files_tmp us_files_tmp ca_files_tmp

bankdata: $(at_files) $(ch_files) $(de_files) $(us_files) $(ca_files)

all-local: bankdata

dist-local: at.tar.bz2 ch.tar.bz2 de.tar.bz2 us.tar.bz2 ca.tar.bz2

CLEANFILES = $(at_files) $(ch_files) $(de_files) $(ca_files) $(us_files)

if IS_WINDOWS
# Doesn't link on windows; too lazy to fix this for now.
noinst_PROGRAMS=
else
noinst_PROGRAMS=mkdeinfo
mkdeinfo_SOURCES=mkdeinfo.c
mkdeinfo_LDADD = $(aqbanking_internal_libs) $(gwenhywfar_libs)
endif
# IS_WINDOWS

sources:
	for f in $(libbankinfo_la_SOURCES); do \
	  echo $(subdir)/$$f >>$(top_srcdir)/i18nsources; \
	done
