SUBDIRS=doc m4 src

EXTRA_DIST=AUTHORS COPYING ChangeLog INSTALL NEWS README \
 aqbanking.m4 \
 aqbanking.spec.in Doxyfile.in version.h.in


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

srcdoc: Doxyfile
	doxygen

install-srcdoc: $(PACKAGE).tag
	test -d "$(DESTDIR)@docpath@/$(PACKAGE)" || \
	  mkdir -p "$(DESTDIR)@docpath@/$(PACKAGE)"
	cp apidoc/* "$(DESTDIR)@docpath@/$(PACKAGE)"
	cp "$(PACKAGE).tag" "$(DESTDIR)@docpath@/"
	if test -x "$(DESTDIR)@docpath@/$(PACKAGE)/installdox"; then \
	  ( cd "$(DESTDIR)@docpath@/$(PACKAGE)" && \
	    for ff in ${DOXYGEN_TAGFILES}; do \
	     ./installdox -q -l "`basename $$ff`@../`basename $$ff | $(SED) -e s/.tag//`";\
	    done ) \
	fi


rpm:    $(PACKAGE).spec dist
	rpmbuild="rpm" && \
	if [ `rpm --version | awk '{ print $$3 }'` > /dev/null ]; then rpmbuild="rpmbuild"; fi && \
	$$rpmbuild -ta $(PACKAGE)-$(VERSION).tar.gz

