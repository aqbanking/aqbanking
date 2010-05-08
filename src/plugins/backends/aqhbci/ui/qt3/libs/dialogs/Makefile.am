SUBDIRS=wizard

INCLUDES = $(gwenhywfar_includes) $(qt3_includes) \
  -I$(aqbanking_symlinkdir) \
  -I$(srcdir)/.. -I$(srcdir)/wizard \
  $(GWENGUI_QT3_CFLAGS) \
  -I.. -Iwizard


UI_FILES= \
 cfgtabpageuserhbci.ui \
 cfgtabpageaccounthbci.ui \
 selectmode.ui

EXTRA_DIST= $(UI_FILES)

noinst_LTLIBRARIES=libdialogs.la

nodist_noinst_HEADERS=\
 cfgtabpageuserhbci.ui.h \
 cfgtabpageaccounthbci.ui.h \
 selectmode.ui.h

noinst_HEADERS=\
 cfgtabpageuserhbci.h \
 cfgtabpageaccounthbci.h \
 selectmode.h \
 userwizard.h


nodist_libdialogs_la_SOURCES=\
 cfgtabpageuserhbci.ui.cpp \
 cfgtabpageaccounthbci.ui.cpp \
 selectmode.ui.cpp


libdialogs_la_SOURCES=\
 cfgtabpageuserhbci.cpp \
 cfgtabpageaccounthbci.cpp \
 selectmode.cpp \
 userwizard.cpp

MOC_FILES=\
 cfgtabpageaccounthbci.moc \
 cfgtabpageaccounthbci.ui.moc \
 cfgtabpageuserhbci.moc \
 cfgtabpageuserhbci.ui.moc \
 selectmode.ui.moc
 

libdialogs_la_LIBADD=wizard/libwizard.la


BUILT_SOURCES = $(nodist_libdialogs_la_SOURCES) $(nodist_noinst_HEADERS) $(MOC_FILES)
CLEANFILES = $(BUILT_SOURCES)


sources:
	for f in $(libdialogs_la_SOURCES) $(foreach uifile,$(UI_FILES),$(uifile).cpp); do \
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
	for d in $(SUBDIRS); do \
	  $(MAKE) -C $$d qt4-port; \
	done
	for A in $(libdialogs_la_SOURCES) $(noinst_HEADERS); do \
	 $(YES) | $(QT3TO4) $$A; \
	done
	rm -f $(BUILT_SOURCES)
# For qt3 compatibility of qt4
DEFS += -DQT3_SUPPORT

built_sources: $(BUILT_SOURCES)
	for d in $(SUBDIRS); do \
	  $(MAKE) -C $$d built_sources; \
	done ;

