INCLUDES = $(all_includes)
EXTRA_DIST=\
  dbb.conf hbci.conf atblz.conf fedachdir.conf bcbankenstamm.conf \
  README at.tar.bz2 ch.tar.bz2 de.tar.bz2 us.tar.bz2

bankinfoplugindir = $(aqbanking_plugindir)/bankinfo
bankinfodatadir = $(aqbanking_pkgdatadir)/bankinfo

DEFS += -DPLUGIN_DATADIR=\"$(bankinfodatadir)\"

noinst_LTLIBRARIES=libbankinfo.la


noinst_HEADERS=\
 generic_p.h \
 generic_l.h

libbankinfo_la_SOURCES=\
 generic.c

libbankinfo_la_LIBADD = -L$(top_builddir)/src/libs/aqbanking -laqbanking $(gwenhywfar_libs)
libbankinfo_la_LDFLAGS = $(STRIPALL)

at_files=at/blz.idx at/bic.idx at/namloc.idx at/banks.data
ch_files=ch/blz.idx ch/bic.idx ch/namloc.idx ch/banks.data
de_files=de/blz.idx de/bic.idx de/namloc.idx de/banks.data
us_files=us/blz.idx us/bic.idx us/namloc.idx us/banks.data

at.tar.bz2:
	./mkdeinfo build-at data/blz-at.csv tmp.banks
	test -d at || mkdir -p at
	./mkdeinfo install tmp.banks at
	tar cf at.tar at && bzip2 -9 at.tar
	rm -rf tmp.banks

$(at_files): at.tar.bz2
	test -d at || mkdir -p at
	bunzip2 -k -f at.tar.bz2 && tar xmf at.tar && rm -f at.tar

at-data: $(at_files)

install-at: $(at_files)
	$(mkinstalldirs) $(DESTDIR)$(bankinfodatadir)/at
	$(INSTALL_DATA) $(at_files) $(DESTDIR)$(bankinfodatadir)/at


ch.tar.bz2:
	./mkdeinfo build-ch data/bcbankenstamm.csv tmp.banks
	test -d ch || mkdir -p ch
	./mkdeinfo install tmp.banks ch
	tar cf ch.tar ch && bzip2 -9 ch.tar
	rm -rf tmp.banks

$(ch_files): ch.tar.bz2
	test -d ch || mkdir -p ch
	bunzip2 -k -f ch.tar.bz2 && tar xmf ch.tar && rm -f ch.tar

ch-data: $(ch_files)

install-ch: $(ch_files)
	$(mkinstalldirs) $(DESTDIR)$(bankinfodatadir)/ch
	$(INSTALL_DATA) $(ch_files) $(DESTDIR)$(bankinfodatadir)/ch


de.tar.bz2:
	./mkdeinfo build data/blz.csv data/fints_institute.csv tmp.banks
	test -d de || mkdir -p de
	./mkdeinfo install tmp.banks de
	tar cf de.tar de && bzip2 -9 de.tar
	rm -rf tmp.banks

$(de_files): de.tar.bz2
	test -d de || mkdir -p de
	bunzip2 -k -f de.tar.bz2 && tar xmf de.tar && rm -f de.tar

de-data: $(de_files)

install-de: $(de_files)
	$(mkinstalldirs) $(DESTDIR)$(bankinfodatadir)/de
	$(INSTALL_DATA) $(de_files) $(DESTDIR)$(bankinfodatadir)/de


us.tar.bz2:
	./mkdeinfo build-am data/bankinfo us data/FedACHdir.txt tmp.banks
	test -d us || mkdir -p us
	./mkdeinfo install tmp.banks us
	tar cf us.tar us && bzip2 -9 us.tar
	rm -rf tmp.banks

$(us_files): us.tar.bz2
	test -d us || mkdir -p us
	bunzip2 -k -f us.tar.bz2 && tar xmf us.tar && rm -f us.tar

us-data: $(us_files)

install-us: $(us_files)
	$(mkinstalldirs) $(DESTDIR)$(bankinfodatadir)/us
	$(INSTALL_DATA) $(us_files) $(DESTDIR)$(bankinfodatadir)/us

bankdata: at-data ch-data de-data us-data

install-bankdata: install-at install-ch install-de install-us

install-data-local: install-bankdata

all-local: bankdata

dist-local: at.tar.bz2 ch.tar.bz2 de.tar.bz2 us.tar.bz2

noinst_PROGRAMS=mkdeinfo
mkdeinfo_SOURCES=mkdeinfo.c
mkdeinfo_LDADD = -L$(top_builddir)/src/libs/aqbanking -laqbanking $(gwenhywfar_libs)

sources:
	for f in $(de_la_SOURCES); do \
	  echo $(subdir)/$$f >>$(top_srcdir)/i18nsources; \
	done
