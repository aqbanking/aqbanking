SUBDIRS=doc m4 src po

EXTRA_DIST=AUTHORS COPYING ChangeLog INSTALL NEWS README \
 aqbanking.m4 \
 aqbanking.spec.in aqbanking.spec Doxyfile.in version.h.in \
 i18nsources


bin_SCRIPTS=aqbanking-config
M4dir=${prefix}/share/aclocal
M4_DATA=aqbanking.m4

ACLOCAL_AMFLAGS = $(ADD_ACLOCAL_FLAGS) -I m4

iheaderdir=${includedir}/aqbanking
iheader_HEADERS=\
  version.h


aqbanking-config: aqbanking-config.in
	echo "#! /bin/sh" >aqbanking-config && \
	echo 'dir="@prefix@"' >>aqbanking-config && \
	cat aqbanking-config.in >>aqbanking-config && \
	chmod a+x aqbanking-config

listdoc.h: @AQ_HEADER_FILES@
	mklistdoc -v @AQ_HEADER_FILES@ >listdoc.h

srcdoc: Doxyfile listdoc.h
	doxygen

install-srcdoc: srcdoc
	test -d "$(DESTDIR)@docpath@/$(PACKAGE)" || \
	  mkdir -p "$(DESTDIR)@docpath@/$(PACKAGE)"
	cp apidoc/* "$(DESTDIR)@docpath@/$(PACKAGE)"
	cp "$(PACKAGE).tag" "$(DESTDIR)@docpath@/"
	if test -x "$(DESTDIR)@docpath@/$(PACKAGE)/installdox"; then \
	  cd "$(DESTDIR)@docpath@/$(PACKAGE)" && \
	   ID_OPT="-q" ; \
	   for ff in ${DOXYGEN_TAGFILES}; do \
	    ID_OPT="$$ID_OPT -l `basename $$ff`@../`basename $$ff | $(SED) -e s/.tag//`";\
	   done ;\
	   ( cd "$(DESTDIR)@docpath@/$(PACKAGE)" && \
	      ./installdox -q $$ID_OPT \
	   ); \
	fi


rpm:    $(PACKAGE).spec dist
	rpmbuild="rpm" && \
	if [ `rpm --version | awk '{ print $$3 }'` > /dev/null ]; then rpmbuild="rpmbuild"; fi && \
	$$rpmbuild -ta $(PACKAGE)-$(VERSION).tar.gz

# Rule for using the Inno Setup compiler. The path to iscc.exe is set
# manually here for now.
ISCC = "C:\Programme\Inno Setup 4\iscc.exe"
wintmpdir = win32-tmp
setup: aqbanking.iss
	mkdir -p $(wintmpdir)
	make prefix="`pwd`/$(wintmpdir)" \
	     gwenhywfar_plugins="`pwd`/$(wintmpdir)/gwen" install \
	 && $(ISCC) aqbanking.iss \
	 && rm -rf $(wintmpdir)

sources:
	rm -f i18nsources
	make -C src sources

if USE_I18N

all-local: sources catalog

I18NFILES=$(shell cat i18nsources)
I18NLINGUAS=$(shell ls po/*.po)

messages: $(I18NFILES);
	$(XGETTEXT) -C -c -ki18n -ktr2i18n -kI18N \
	  -kI18N_NOOP -ktranslate -kaliasLocale \
          -ktr -ktrUtf8 \
	  '--msgid-bugs-address=openhbci-general@lists.sourceforge.net' \
	  -o po/$(PACKAGE).pot \
          $(I18NFILES);

merge: messages
	@catalogs=`find ./po -name "*.po"`; \
	for cat in $$catalogs; do \
	echo $$cat $$name; \
	msgmerge -o $$cat.new $$cat ./po/$(PACKAGE).pot ; \
	if test -s $$cat.new; then \
	  grep -v "\"POT-Creation" $$cat.new > $$cat.new.2 ; \
	  grep -v "\"POT-Creation" $$cat >> $$cat.new.1; \
	  if diff $$cat.new.1 $$cat.new.2; then \
		rm $$cat.new;  \
	  else  \
		mv $$cat.new $$cat ; \
	fi; \
	rm -f $$cat.new.1 $$cat.new.2 ;\
	fi ;\
	done

catalog: merge $(I18NLINGUAS)
	 LIST=`find ./po -name "*.po"`; \
	 for i in $$LIST; do \
	  file2=`echo $$i | sed -e "s#\.po#\.mo#"`; \
	  echo $$file2; \
	  msgfmt -o $$file2 $$i || touch $$i; \
	 done ;

install-data-local:
	LIST=`(cd po && ls *.mo)`; \
	for i in $$LIST; do \
	file2=`echo $$i | sed "s/\.mo//"`; \
        echo installing $$i; \
        mkdir -p "${DESTDIR}$(prefix)/share/locale/$$file2/LC_MESSAGES"; \
        cp ./po/$$i ${DESTDIR}$(prefix)/share/locale/$$file2/LC_MESSAGES/$(PACKAGE).mo; \
        done


CLEANFILES=i18nsources


endif



