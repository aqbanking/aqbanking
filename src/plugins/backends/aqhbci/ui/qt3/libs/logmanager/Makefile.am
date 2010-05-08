INCLUDES = \
  -I$(aqbanking_symlinkdir) \
  -I$(srcdir)/../widgets -I$(srcdir)/.. \
  -I../widgets -I.. \
  $(GWENGUI_QT3_CFLAGS) \
  $(all_includes)

DEFS += -DAQHBCI_DATAFOLDER=\"$(aqbanking_pkgdatadir)/backends/aqhbci\"

UI_FILES= \
  logmanager.ui

EXTRA_DIST= $(UI_FILES)

noinst_LTLIBRARIES=liblogmanager.la

nodist_noinst_HEADERS=\
  logmanager.ui.h

noinst_HEADERS=\
  logmanager.h \
  error.h \
  pointer.h \
  loganalyzer.h

nodist_liblogmanager_la_SOURCES=\
 logmanager.ui.cpp

liblogmanager_la_SOURCES=\
  logmanager.cpp \
  error.cpp \
  loganalyzer.cpp

liblogmanager_la_LIBADD = $(qbanking_internal_libs) $(aqhbci_internal_libs)

MOC_FILES=\
  logmanager.moc \
  logmanager.ui.moc


BUILT_SOURCES = $(nodist_liblogmanager_la_SOURCES) $(nodist_noinst_HEADERS) $(MOC_FILES)
CLEANFILES = $(BUILT_SOURCES)


sources:
	for f in $(liblogmanager_la_SOURCES) $(foreach uifile,$(UI_FILES),$(uifile).cpp); do \
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
	for A in $(liblogmanager_la_SOURCES) $(noinst_HEADERS); do \
	 $(YES) | $(QT3TO4) $$A; \
	done
	rm -f $(BUILT_SOURCES)
# For qt3 compatibility of qt4
DEFS += -DQT3_SUPPORT

built_sources: $(BUILT_SOURCES)

