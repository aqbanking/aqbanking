AM_CPPFLAGS = -I$(top_srcdir)/src/libs \
  -I$(top_builddir)/src/libs \
  $(gwenhywfar_includes)

AM_CFLAGS=-DBUILDING_AQBANKING @visibility_cflags@

EXTRA_DIST=\
  dbb.conf hbci.conf atblz.conf fedachdir.conf bcbankenstamm.conf \
  kidaten.conf README de.tar.bz2

bankinfoplugindir = $(aqbanking_plugindir)/bankinfo
bankinfodatadir = $(aqbanking_pkgdatadir)/bankinfo

#DEFS += -DPLUGIN_DATADIR=\"$(bankinfodatadir)\"


MKDEINFO=./mkdeinfo

noinst_LTLIBRARIES=libbankinfo_generic.la
noinst_HEADERS=\
 generic_p.h \
 generic_l.h

libbankinfo_generic_la_SOURCES=generic.c

de_files=de/blz.idx de/bic.idx de/namloc.idx de/banks.data

#atbankdatadir = $(bankinfodatadir)/at
#atbankdata_DATA = $(at_files)

debankdatadir = $(bankinfodatadir)/de
debankdata_DATA = $(de_files)

#chbankdatadir = $(bankinfodatadir)/ch
#chbankdata_DATA = $(ch_files)

#usbankdatadir = $(bankinfodatadir)/us
#usbankdata_DATA = $(us_files)

#cabankdatadir = $(bankinfodatadir)/ca
#cabankdata_DATA = $(ca_files)

de.tar.bz2:
	$MKDEINFO build data/de/blz.csv data/de/fints_institute.csv tmp.banks
	test -d de || mkdir -p de
	if test -f data/de/aux.conf; then \
	  $MKDEINFO update tmp.banks data/de/aux.conf tmp.banks; \
	fi
	$MKDEINFO install tmp.banks de
	tar cf de.tar de && bzip2 -9 de.tar
	rm -rf tmp.banks

$(de_files): de_files_tmp

de_files_tmp: de.tar.bz2
	test -d de || mkdir -p de
	bunzip2 -k -c $< | tar xmof -
	touch $@

.INTERMEDIATE: de_files_tmp

bankdata: $(de_files)

all-local: bankdata

dist-local: de.tar.bz2

CLEANFILES = $(at_files) $(ch_files) $(de_files) $(ca_files) $(us_files)

sources:
	for f in $(libbankinfo_la_SOURCES); do \
	  echo $(subdir)/$$f >>$(top_srcdir)/i18nsources; \
	done


cppcheck:
	for f in $(libbankinfo_la_SOURCES); do \
	  cppcheck --force $$f ; \
	done
	for d in $(SUBDIRS); do \
	  $(MAKE) -C $$d cppcheck; \
	done



