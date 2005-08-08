SUBDIRS=prg widgets dialogs views po

SUBPACKAGE=kbanking

INCLUDES = \
  -I$(top_builddir)/headers \
  -I$(srcdir)/dialogs -I$(srcdir)/widgets -I$(srcdir)/prg -I$(srcdir)/views \
  -Idialogs -Iwidgets -Iprg -Iviews \
  $(all_includes)

DEFS += -DDATADIR=\"$(datadir)/$(SUBPACKAGE)\"

lib_LTLIBRARIES=libkbanking.la

bin_SCRIPTS=kbanking-config
M4dir=${prefix}/share/aclocal
dist_M4_DATA=kbanking.m4

iheaderdir=${includedir}/kbanking
iheader_HEADERS=\
 flagstaff.h \
 kbanking_p.h \
 kbanking.h \
 waitcallback.h 

libkbanking_la_SOURCES=\
 flagstaff.cpp \
 kbanking.cpp \
 waitcallback.cpp

nodist_libkbanking_la_SOURCES=\
 flagstaff.moc.cpp

kbanking-config: kbanking-config.in
	echo "#! /bin/sh" >kbanking-config && \
	echo 'dir="@prefix@"' >>kbanking-config && \
	cat kbanking-config.in >>kbanking-config && \
	chmod a+x kbanking-config

BUILT_SOURCES =$(nodist_libkbanking_la_SOURCES) version.h i18nsources
CLEANFILES=$(BUILT_SOURCES) i18nsources kbanking-config

libkbanking_la_LIBADD=\
 prg/libprg.la \
 widgets/libwidgets.la \
 dialogs/libdialogs.la \
 views/libviews.la \
 $(kde3_libs) -lkdeui -lkdecore -lkio \
 -L$(top_builddir)/src/libs/aqbanking $(aqbanking_internal_libs) $(gwenhywfar_libs)

libkbanking_la_LDFLAGS=\
 -version-info @KBANKING_SO_CURRENT@:@KBANKING_SO_REVISION@:@KBANKING_SO_AGE@


sources:
	for f in $(libkbanking_la_SOURCES) $(UI_FILES); do \
	  echo $(subdir)/$$f >>i18nsources; \
	done
	for d in $(SUBDIRS); do \
	  make -C $$d sources; \
	done ; \
	ofiles="`cat i18nsources`"; \
	files=""; \
	rm -f i18nsources.tmp 2>/dev/null ;\
	for i in $$ofiles; do \
	  echo $$i | $(SED) "s:$(subdir)/::g" >>i18nsources.tmp ; \
	done ; \
	rm -f i18nsources ; \
	mv i18nsources.tmp i18nsources

i18nsources: sources


if USE_I18N

all-local: sources

I18NFILES=$(shell cat i18nsources)
I18NLINGUAS=$(shell ls po/*.po)

messages:
	$(XGETTEXT) -C -c -ki18n -ktr2i18n -kI18N \
	  -kI18N_NOOP -ktranslate -kaliasLocale \
          -ktr -ktrUtf8 -k_tr\
	  '--msgid-bugs-address=openhbci-general@lists.sourceforge.net' \
	  --from-code=ISO-8859-15 \
	  -o po/$(SUBPACKAGE).pot \
          $(I18NFILES);

merge: messages
	@catalogs=`find ./po -name "*.po"`; \
	for cat in $$catalogs; do \
	echo $$cat $$name; \
	msgmerge -o $$cat.new $$cat ./po/$(SUBPACKAGE).pot ; \
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

catalog: $(I18NLINGUAS)
	 LIST=`find ./po -name "*.po"`; \
	 for i in $$LIST; do \
	  file2=`echo $$i | sed -e "s#\.po#\.qm#"`; \
	  echo $$file2; \
	  msgfmt -o $$file2 --qt $$i || touch $$i; \
	 done ;

install-data-local: catalog
	LIST=`(cd po && ls *.qm)`; \
        mkdir -p "${DESTDIR}$(prefix)/share/$(SUBPACKAGE)/i18n"; \
	for i in $$LIST; do \
	  file2=`echo $$i | sed "s/\.qm//"`; \
	  echo installing $$i; \
	  cp ./po/$$i ${DESTDIR}$(prefix)/share/$(SUBPACKAGE)/i18n/; \
        done

po: catalog

CLEANFILES += i18nsources

endif



%.moc.cpp: %.h
	@qt3_moc@ -o $@ $<


clean-local:
	rm -f *.ui.{h.cpp} *.moc.* >/dev/null

clean-ui: 
	rm -f *.ui.{cpp,h}



