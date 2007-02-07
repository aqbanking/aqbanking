SUBDIRS=admin doc m4 src bindings po tutorials

EXTRA_DIST=AUTHORS COPYING ChangeLog INSTALL NEWS README \
 aqbanking.spec.in aqbanking.spec \
 aqbanking.iss.in aqbanking.iss \
 Doxyfile.in version.h.in system.h.in \
 i18nsources qtsources \
 mksymlinks.sh.in mksymlinks.sh

idatadir=${datadir}/doc/aqbanking
idata_DATA=AUTHORS COPYING ChangeLog README

bin_SCRIPTS=aqbanking-config
aclocaldir = $(datadir)/aclocal
dist_aclocal_DATA = aqbanking.m4

ACLOCAL_AMFLAGS = $(ADD_ACLOCAL_FLAGS) -I m4

iheaderdir=${includedir}/aqbanking
iheader_HEADERS=\
  version.h system.h

CLEANFILES = aqbanking-config aqbanking-config.in

aqbanking-config: aqbanking-config.in
	echo "#! /bin/sh" >aqbanking-config && \
	echo 'dir="@prefix@"' >>aqbanking-config && \
	cat aqbanking-config.in >>aqbanking-config && \
	chmod a+x aqbanking-config

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = aqbanking.pc

listdoc.h:
	mklistdoc -v `find headers -name '*.h' ` >listdoc.h

srcdoc: Doxyfile listdoc.h
	if test -d apidoc; then rm -Rf apidoc; fi
	mkdir -p apidoc/html/aqbanking-@AQBANKING_VERSION_RELEASE_STRING@
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
	for A in `find src/plugins -name *.la`; do \
	  grep -v relink_command $${A} > $${A}.tmp && \
	    mv $${A}.tmp $${A}; \
	done
	$(MAKE) DESTDIR="`pwd`/$(wintmpdir)" \
	     aqbanking_pkgdatadir="/share/$(PACKAGE)" \
	     gwenhywfar_plugins="/gwen" \
	     WIN_PATH_WINDOWS_MINGW="/windows" \
	     install \
	 && for A in `find $(wintmpdir) -name '*.exe'`; do \
		strip $${A}; \
	    done \
	 && $(ISCC) aqbanking.iss \
	 && rm -rf $(wintmpdir)

if USE_I18N

# We directly read that file list. For old make programs, you would
# set this to an empty variable, because they are only needed by the
# developers and release technicians.
I18NFILES=$(shell cat $(srcdir)/i18nsources $(srcdir)/qtsources)
# Current build rule:
sources:
	rm -f $(srcdir)/i18nsources $(srcdir)/qtsources
	$(MAKE) -C $(top_srcdir)/src sources
$(srcdir)/i18nsources $(srcdir)/qtsources:
	$(MAKE) sources

# Could also be solved this way: Read the I18NFILES variable from file
#include $(srcdir)/i18nsources
# (Needs the following build rule:)
#sources:
#	rm -f $(srcdir)/i18nsources
#	$(MAKE) -C $(top_srcdir)/src sources
#	mv $(srcdir)/i18nsources $(srcdir)/i18nsources.tmp
#	echo "I18NFILES = \\" > $(srcdir)/i18nsources
#	$(SED) 's!\(^.*$$\)!$$(top_srcdir)/\1 \\!' \
#	  $(srcdir)/i18nsources.tmp >> $(srcdir)/i18nsources
#	echo '$$(top_srcdir)/config.h' >> $(srcdir)/i18nsources
#	rm -f $(srcdir)/i18nsources.tmp
#	$(MAKE) Makefile

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

po/$(PACKAGE).pot: i18nsources qtsources $(I18NFILES)
	if test -n "$(I18NFILES)"; then \
	  $(XGETTEXT) -C -c -ki18n -ktr2i18n -kI18N \
	    -kI18N_NOOP -ktranslate -kaliasLocale \
            -ktr -ktrUtf8 \
	    '--msgid-bugs-address=openhbci-general@lists.sourceforge.net' \
	    -o po/$(PACKAGE).pot \
	    $(I18NFILES); \
	fi

# This rule is not called automatically. It is necessary only for
# the translators, so translators only call this manually.
merge: po/$(PACKAGE).pot
	@catalogs=$(I18NLINGUAS); \
	for cat in $$catalogs; do \
	echo msgmerge -o $$cat.new $$cat ./po/$(PACKAGE).pot ; \
	msgmerge -o $$cat.new $$cat ./po/$(PACKAGE).pot ; \
	if test -s $$cat.new; then \
	  grep -v "\"POT-Creation" $$cat.new | grep -v '^#' > $$cat.new.2 ; \
	  grep -v "\"POT-Creation" $$cat | grep -v '^#' >> $$cat.new.1; \
	  if diff $$cat.new.1 $$cat.new.2; then \
		echo "  $$cat unchanged; leaving existing po file untouched."; \
		rm $$cat.new;  \
	  else  \
		mv $$cat.new $$cat ; \
	fi; \
	rm -f $$cat.new.1 $$cat.new.2 ;\
	fi ;\
	done

catalogs: $(I18NLINGUAS)
	@for i in $(ALL_LINGUAS); do \
	  filelingua=$(srcdir)/po/$$i.po; \
	  filecat=$(top_builddir)/po/$$i.mo; \
	  fileqm=$(top_builddir)/po/$$i.qm; \
	  echo "Creating translation catalogs for language \"$$i\""; \
	  $(MSGFMT) -o $${filecat} $${filelingua} || touch $${filelingua}; \
	  if test $(MSGFMT_SUPPORTS_QT) = "yes"; then \
	    $(MSGFMT) -o $${fileqm} --qt $${filelingua} || touch $${filelingua}; \
	  else \
	    echo "$(MSGFMT) does not have the option --qt; ignoring qt translations"; \
	  fi; \
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
uninstall-mo-catalogs:
	@$(NORMAL_UNINSTALL)
	@for i in $(ALL_LINGUAS); do \
	  echo " rm -f '$(DESTDIR)$(localedir)/$$i/LC_MESSAGES/$(PACKAGE).mo'"; \
	  rm -f "$(DESTDIR)$(localedir)/$$i/LC_MESSAGES/$(PACKAGE).mo"; \
	done


if MAKE_QT_CATS

qtlocaledir = $(pkgdatadir)/i18n
install-qt-catalogs: catalogs
	$(mkinstalldirs) "$(DESTDIR)$(qtlocaledir)";
	@if test $(MSGFMT_SUPPORTS_QT) = "yes"; then \
	  for i in $(ALL_LINGUAS); do \
	    fileqm=$(top_builddir)/po/$$i.qm; \
            echo "Installing translation catalog for language \"$$i\""; \
            $(INSTALL_DATA) $${fileqm} $(DESTDIR)$(qtlocaledir)/$$i.qm; \
          done \
	fi
uninstall-qt-catalogs:
	@$(NORMAL_UNINSTALL)
	@for i in $(ALL_LINGUAS); do \
	  echo " rm -f '$(DESTDIR)$(qtlocaledir)/$$i.qm'"; \
	  rm -f "$(DESTDIR)$(qtlocaledir)/$$i.qm"; \
	done

install-data-local: install-qt-catalogs install-mo-catalogs
uninstall-local: uninstall-qt-catalogs uninstall-mo-catalogs

else

install-data-local: install-mo-catalogs
uninstall-local: uninstall-mo-catalogs

endif # if MAKE_QT_CATS

CLEANFILES += $(I18NCATALOGS)

endif # if USE_I18N


# Extra rule instead of "all" which creates the auto-generated headers
# *first* and only then starts "$(MAKE) all". This avoids duplicate
# compilation.
quick: config.h version.h system.h
	$(MAKE)

# Rule for only building the source rpm
srpm: $(PACKAGE).spec dist
	rpmbuild="rpm" && \
	if [ `rpm --version | awk '{ print $$3 }'` > /dev/null ]; then rpmbuild="rpmbuild"; fi && \
	$$rpmbuild --nodeps -ts $(PACKAGE)-$(VERSION).tar.gz

# This section is for a qt4 environment
qt4-port:
	for d in src/frontends/qbanking src/plugins/backends/aqhbci/ui/qt3 src/plugins/backends/aqdtaus/ui/qt3 src/plugins/backends/aqofxconnect/ui/qt3; do \
	  $(MAKE) -C $$d qt4-port; \
	done

BUILT_SOURCES = $(iheader_HEADERS) sl-headers
CLEANFILES += $(BUILT_SOURCES)

symlinks: mksymlinks.sh
	${SHELL} mksymlinks.sh $(srcdir) $(builddir)

sl-headers:
	$(MAKE) -C src built_sources
	${SHELL} mksymlinks.sh $(srcdir) $(builddir)

# Call aqbanking's testcases plus the unittests from the
# moneysplash module
py-check:
	cd ../moneysplash/unittests && \
	  PYTHONPATH=$(pyexecdir) $(PYTHON) test_1.py

# Call this to have Qt's uic files and glade files cleaned up from
# bad elements
code-cleanup:
	-admin/fixuifiles
	-admin/glade-fixup
