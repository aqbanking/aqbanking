INCLUDES = \
  -I$(top_builddir)/headers \
  $(qt4_includes) $(gwenhywfar_includes) 

DEFS += -DPKGDATADIR=\"$(pkgdatadir)\" \
       -DQ4BANKING_HELPDIR=\"@q4banking_helpdir@\"

#noinst_LTLIBRARIES=libqbhelp.la

UI_FILES=\
 qbhelpbrowser.ui

noinst_HEADERS=\
 qbhelpbrowser.h

nodist_noinst_HEADERS=\
 qbhelpbrowser.ui.h

MOC_FILES=\
 qbhelpbrowser.moc


bin_PROGRAMS = q4b-help@Q4BANKING_SO_EFFECTIVE@
q4b_help@Q4BANKING_SO_EFFECTIVE@_SOURCES = qb_help.cpp qbhelpbrowser.cpp
q4b_help@Q4BANKING_SO_EFFECTIVE@_LDADD= \
  $(qt4_libs) -lQtCore -lQt3Support \
  $(gwenhywfar_libs)

BUILT_SOURCES = $(MOC_FILES) $(nodist_libqbhelp_la_SOURCES) $(nodist_noinst_HEADERS)
CLEANFILES = $(BUILT_SOURCES)
EXTRA_DIST = $(UI_FILES)


%.moc: %.h
	$(qt4_moc) -o $@ $<

%.ui.h: %.ui
	$(qt4_uic) $< -o $@

SUFFIXES = .ui .ui.h .moc

sources:
	for f in $(libqbhelp_la_SOURCES); do \
	  echo $(subdir)/$$f >>$(top_srcdir)/qtsources; \
	done
	for d in $(SUBDIRS); do \
	  $(MAKE) -C $$d sources; \
	done

# For qt3 compatibility of qt4
DEFS += -DQT3_SUPPORT

built_sources: $(BUILT_SOURCES)

