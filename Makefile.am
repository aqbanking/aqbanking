SUBDIRS=doc m4 src bindings po

EXTRA_DIST=AUTHORS COPYING ChangeLog INSTALL NEWS README \
 aqbanking.spec.in aqbanking.spec \
 aqbanking.iss.in aqbanking.iss \
 Doxyfile.in version.h.in \
 i18nsources

idatadir=${datadir}/doc/aqbanking
idata_DATA=AUTHORS COPYING ChangeLog README

bin_SCRIPTS=aqbanking-config
aclocaldir = $(datadir)/aclocal
dist_aclocal_DATA = aqbanking.m4

ACLOCAL_AMFLAGS = $(ADD_ACLOCAL_FLAGS) -I m4

iheaderdir=${includedir}/aqbanking
iheader_HEADERS=\
  version.h

CLEANFILES = aqbanking-config aqbanking-config.in

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
ISCC = "C:\Programme\Inno Setup 5\iscc.exe"
wintmpdir = win32-tmp
setup: aqbanking.iss
	mkdir -p $(wintmpdir)
	make prefix="`pwd`/$(wintmpdir)" \
	     aqbanking_pkgdatadir="`pwd`/$(wintmpdir)/share/$(PACKAGE)" \
	     gwenhywfar_plugins="`pwd`/$(wintmpdir)/gwen" \
	     WIN_PATH_WINDOWS_MINGW="`pwd`/$(wintmpdir)/windows" \
	     install \
	 && $(ISCC) aqbanking.iss \
	 && rm -rf $(wintmpdir)

if USE_I18N

# We directly read that file list. For old make programs, you would
# set this to an empty variable, because they are only needed by the
# developers and release technicians.
I18NFILES=$(shell cat $(srcdir)/i18nsources $(srcdir)/qtsources)
# Current build rule:
sources:
	rm -f $(srcdir)/i18nsources
	make -C $(top_srcdir)/src sources
$(srcdir)/i18nsources $(srcdir)/qtsources:
	make sources

# Could also be solved this way: Read the I18NFILES variable from file
#include $(srcdir)/i18nsources
# (Needs the following build rule:)
#sources:
#	rm -f $(srcdir)/i18nsources
#	make -C $(top_srcdir)/src sources
#	mv $(srcdir)/i18nsources $(srcdir)/i18nsources.tmp
#	echo "I18NFILES = \\" > $(srcdir)/i18nsources
#	$(SED) 's!\(^.*$$\)!$$(top_srcdir)/\1 \\!' \
#	  $(srcdir)/i18nsources.tmp >> $(srcdir)/i18nsources
#	echo '$$(top_srcdir)/config.h' >> $(srcdir)/i18nsources
#	rm -f $(srcdir)/i18nsources.tmp
#	make Makefile

# The list of available languages (makes file name transformations much easier)
ALL_LINGUAS = $(basename $(notdir $(shell ls $(srcdir)/po/*.po)))
# For old make programs, the locales would directly be written here:
#ALL_LINGUAS = de

# List of po files. Only necessary in the dependency of the catalogs
# rule. For old make programs, you would simpy set this to an empty
# variable.
I18NLINGUAS = $(foreach lang,$(ALL_LINGUAS),$(srcdir)/po/$(lang).po)

# List of mo files. Only necessary for cleanup. For old make programs,
# you would simply set this to an empty variable.
I18NCATALOGS = $(foreach lang,$(ALL_LINGUAS),$(top_builddir)/po/$(lang).mo)
I18NCATALOGS += $(foreach lang,$(ALL_LINGUAS),$(top_builddir)/po/$(lang).qm)

po/$(PACKAGE).pot: $(I18NFILES)
	if test -n "$(I18NFILES)"; then \
	  $(XGETTEXT) -C -c -ki18n -ktr2i18n -kI18N \
	    -kI18N_NOOP -ktranslate -kaliasLocale \
            -ktr -ktrUtf8 \
	    '--msgid-bugs-address=openhbci-general@lists.sourceforge.net' \
	    -o po/$(PACKAGE).pot \
	    $(I18NFILES); \
	fi

# This rule is called automatically at install time. However, it is
# necessary only for the translators, so maybe it shouldn't be called
# at install time.
merge: po/$(PACKAGE).pot
	@catalogs=$(I18NLINGUAS); \
	for cat in $$catalogs; do \
	echo msgmerge -o $$cat.new $$cat ./po/$(PACKAGE).pot ; \
	msgmerge -o $$cat.new $$cat ./po/$(PACKAGE).pot ; \
	if test -s $$cat.new; then \
	  grep -v "\"POT-Creation" $$cat.new > $$cat.new.2 ; \
	  grep -v "\"POT-Creation" $$cat >> $$cat.new.1; \
	  if diff $$cat.new.1 $$cat.new.2; then \
		echo "  $$cat unchanged; leaving existing po file untouched."; \
		rm $$cat.new;  \
	  else  \
		mv $$cat.new $$cat ; \
	fi; \
	rm -f $$cat.new.1 $$cat.new.2 ;\
	fi ;\
	done

MSGFMT = msgfmt
catalogs: $(I18NLINGUAS)
	@for i in $(ALL_LINGUAS); do \
	  filelingua=$(srcdir)/po/$$i.po; \
	  filecat=$(top_builddir)/po/$$i.mo; \
	  fileqm=$(top_builddir)/po/$$i.qm; \
	  echo "Creating translation catalogs for language \"$$i\""; \
	  $(MSGFMT) -o $${filecat} $${filelingua} || touch $${filelingua}; \
	  $(MSGFMT) -o $${fileqm} --qt $${filelingua} || touch $${filelingua}; \
	done



localedir = $(datadir)/locale
install-mo-catalogs: catalogs
	@for i in $(ALL_LINGUAS); do \
	  filecat=$(top_builddir)/po/$$i.mo; \
	  fileqm=$(top_builddir)/po/$$i.qm; \
          echo "Installing translation catalog for language \"$$i\""; \
          $(mkinstalldirs) "$(DESTDIR)$(localedir)/$$i/LC_MESSAGES"; \
          $(INSTALL_DATA) $${filecat} $(DESTDIR)$(localedir)/$$i/LC_MESSAGES/$(PACKAGE).mo; \
        done


if MAKE_QT_CATS

qtlocaledir = $(pkgdatadir)/i18n
install-qt-catalogs: catalogs
	$(mkinstalldirs) "$(DESTDIR)$(qtlocaledir)";
	@for i in $(ALL_LINGUAS); do \
	  fileqm=$(top_builddir)/po/$$i.qm; \
          echo "Installing translation catalog for language \"$$i\""; \
          $(INSTALL_DATA) $${fileqm} $(DESTDIR)$(qtlocaledir)/$$i.qm; \
        done

install-data-local: install-qt-catalogs install-mo-catalogs

else

install-data-local: install-mo-catalogs

endif # if MAKE_QT_CATS

CLEANFILES += $(I18NCATALOGS)

endif # if USE_I18N


# Extra rule instead of "all" which creates the auto-generated headers
# *first* and only then starts "make all". This avoids duplicate
# compilation.
quick: config.h version.h
	make

# Rule for only building the source rpm
srpm: $(PACKAGE).spec dist
	rpmbuild="rpm" && \
	if [ `rpm --version | awk '{ print $$3 }'` > /dev/null ]; then rpmbuild="rpmbuild"; fi && \
	$$rpmbuild --nodeps -ts $(PACKAGE)-$(VERSION).tar.gz
