INCLUDES = \
  -I$(top_builddir)/headers \
  -I$(srcdir)/../widgets -I$(srcdir)/.. \
  -I../widgets -I.. \
  $(all_includes)


UI_FILES= \
 editctuser.ui \
 wizard.ui \
 iniletter.ui \
 actionwidget.ui \
 selectfile.ui


EXTRA_DIST= $(UI_FILES)

noinst_LTLIBRARIES=libwizard.la

nodist_noinst_HEADERS=\
 editctuser.ui.h \
 wizard.ui.h \
 iniletter.ui.h \
 actionwidget.ui.h \
 selectfile.ui.h

noinst_HEADERS=\
 actionwidget.h \
 editctuser.h \
 iniletter.h \
 waction.h \
 winfo.h \
 wizard.h \
 a_selectfile.h \
 a_createfile.h \
 a_checkfile.h \
 a_edituser.h \
 a_getcert.h \
 a_getkeys.h \
 a_getsysid.h \
 a_bankiniletter.h \
 a_mkkeys.h \
 a_sendkeys.h \
 a_useriniletter.h \
 a_wait.h \
 a_finished.h \
 a_getaccounts.h \
 w_pintan_new.h \
 w_ddv_import.h \
 w_rdh_import.h \
 w_rdh_new.h \
 w_rdh_new2.h


nodist_libwizard_la_SOURCES=\
 editctuser.ui.cpp \
 wizard.ui.cpp \
 actionwidget.ui.cpp \
 iniletter.ui.cpp \
 selectfile.ui.cpp

libwizard_la_SOURCES=\
 waction.cpp \
 winfo.cpp \
 wizard.cpp \
 actionwidget.cpp \
 editctuser.cpp \
 iniletter.cpp \
 a_selectfile.cpp \
 a_createfile.cpp \
 a_checkfile.cpp \
 a_edituser.cpp \
 a_getcert.cpp \
 a_getkeys.cpp \
 a_bankiniletter.cpp \
 a_mkkeys.cpp \
 a_sendkeys.cpp \
 a_useriniletter.cpp \
 a_getsysid.cpp \
 a_wait.cpp \
 a_finished.cpp \
 a_getaccounts.cpp \
 w_pintan_new.cpp \
 w_ddv_import.cpp \
 w_rdh_import.cpp \
 w_rdh_new.cpp \
 w_rdh_new2.cpp

MOC_FILES=\
 a_bankiniletter.moc \
 a_checkfile.moc \
 a_getaccounts.moc \
 a_getkeys.moc \
 a_getsysid.moc \
 a_mkkeys.moc \
 a_selectfile.moc \
 a_sendkeys.moc \
 a_useriniletter.moc \
 actionwidget.ui.moc \
 editctuser.moc \
 editctuser.ui.moc \
 iniletter.moc \
 iniletter.ui.moc \
 selectfile.ui.moc \
 wizard.moc \
 wizard.ui.moc


BUILT_SOURCES = $(nodist_libwizard_la_SOURCES) $(MOC_FILES) $(nodist_iheader_HEADERS)
CLEANFILES = $(BUILT_SOURCES)


sources:
	for f in $(libwizard_la_SOURCES) $(foreach uifile,$(UI_FILES),$(uifile).cpp); do \
	  echo $(subdir)/$$f >>$(top_srcdir)/qtsources; \
	done
	for d in $(SUBDIRS); do \
	  $(MAKE) -C $$d sources; \
	done


%.moc: %.h
	@qt3_moc@ -o $@ $<


# Build rules for the files that come from the .ui file
%.ui.cpp: %.ui
	$(UIC3) $< -impl $(@:.ui.cpp=.ui.h) -o $@.tmp && \
	echo "#ifdef HAVE_CONFIG_H" >$@ && \
	echo "# include <config.h>" >>$@ && \
	echo "#endif" >>$@ && \
	cat $@.tmp >>$@ && \
	rm $@.tmp && \
	echo "#include \"`basename $<`.moc\"" >>$@

%.ui.h: %.ui
	$(UIC3) $< -o $@
SUFFIXES = .ui .ui.h .ui.cpp .moc.cpp

# This section is for a qt4 environment
qt4-port:
	for A in $(libwizard_la_SOURCES) $(noinst_HEADERS); do \
	 $(YES) | $(QT3TO4) $$A; \
	done
	rm -f $(BUILT_SOURCES)
# For qt3 compatibility of qt4
DEFS += -DQT3_SUPPORT

built_sources: $(BUILT_SOURCES)

