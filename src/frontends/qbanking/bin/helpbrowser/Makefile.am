INCLUDES = \
  -I$(top_builddir)/headers \
  $(qt3_includes) $(all_includes)

DEFS = -DPKGDATADIR=\"$(pkgdatadir)\" \
       -DQBANKING_HELPDIR=\"@qbanking_helpdir@\"

noinst_LTLIBRARIES=libqbhelp.la

UI_FILES=\
 qbhelpbrowser.ui

noinst_HEADERS=\
 qbhelpbrowser.h

nodist_noinst_HEADERS=\
 qbhelpbrowser.ui.h

MOC_FILES=\
 qbhelpbrowser.moc \
 qbhelpbrowser.ui.moc

libqbhelp_la_SOURCES=qbhelpbrowser.cpp

nodist_libqbhelp_la_SOURCES=\
 qbhelpbrowser.ui.cpp


bin_PROGRAMS = qb-help@QBANKING_SO_EFFECTIVE@
qb_help@QBANKING_SO_EFFECTIVE@_SOURCES = qb_help.cpp
qb_help@QBANKING_SO_EFFECTIVE@_LDADD= libqbhelp.la \
  $(qt3_libs) \
  $(gwenhywfar_libs)

BUILT_SOURCES = $(MOC_FILES) $(nodist_libqbhelp_la_SOURCES) $(nodist_noinst_HEADERS)
CLEANFILES = $(BUILT_SOURCES)
EXTRA_DIST = $(UI_FILES)


%.moc: %.h
	$(qt3_moc) -o $@ $<

%.ui.cpp: %.ui
	$(UIC3) $< -impl $(@:.ui.cpp=.ui.h) -o $@.tmp && \
	echo "#ifdef HAVE_CONFIG_H" >$@ && \
	echo "# include <config.h>" >>$@ && \
	echo "#endif" >>$@ && \
	cat $@.tmp >>$@ && \
	rm $@.tmp && \
	echo "#include \"$<.moc\"" >>$@

%.ui.h: %.ui
	$(UIC3) $< -o $@

SUFFIXES = .ui .ui.h .ui.cpp .moc

sources:
	for f in $(libqbhelp_la_SOURCES); do \
	  echo $(subdir)/$$f >>$(top_srcdir)/qtsources; \
	done
	for d in $(SUBDIRS); do \
	  $(MAKE) -C $$d sources; \
	done

# This section is for a qt4 environment
qt4-port:
	for A in $(libqbhelp_la_SOURCES) $(noinst_HEADERS); do \
	 $(YES) | $(QT3TO4) $$A; \
	done
#	rm $(BUILT_SOURCES)
# For qt3 compatibility of qt4
DEFS += -DQT3_SUPPORT

built_sources: $(BUILT_SOURCES)

