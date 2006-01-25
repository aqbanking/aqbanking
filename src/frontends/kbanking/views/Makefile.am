INCLUDES = \
  -I$(top_builddir)/headers \
  -I$(srcdir)/../widgets -I$(srcdir)/../dialogs \
  -I$(srcdir)/../prg -I$(srcdir)/.. \
  $(all_includes)

noinst_LTLIBRARIES=libviews.la

iheaderdir=${includedir}/kbanking
iheader_HEADERS=kbjobview.h jobview.h

nodist_iheader_HEADERS= \
 kbjobview.ui.h

libviews_la_SOURCES=\
 kbjobview.cpp \
 jobview.cpp

nodist_libviews_la_SOURCES=\
  kbjobview.ui.cpp kbjobview.ui.moc.cpp \
  kbjobview.moc.cpp

UI_FILES = kbjobview.ui

EXTRA_DIST = $(UI_FILES)

# Make sure the generated source files are generated before any
# further compiling.
BUILT_SOURCES = $(nodist_libviews_la_SOURCES) $(nodist_iheader_HEADERS)
CLEANFILES = $(BUILT_SOURCES)

sources:
	for f in $(libviews_la_SOURCES) $(foreach uifile,$(UI_FILES),$(uifile).cpp); do \
	  echo $(subdir)/$$f >>$(top_srcdir)/qtsources; \
	done


%.moc.cpp: %.h
	$(qt3_moc) -o $@.tmp $< && \
	echo "#ifdef HAVE_CONFIG_H" >$@ && \
	echo "# include <config.h>" >>$@ && \
	echo "#endif" >>$@ && \
	cat $@.tmp >>$@ && \
	rm $@.tmp


%.ui.cpp: %.ui
	$(UIC3) $< -impl $(@:.ui.cpp=.ui.h) -o $@.tmp && \
	echo "#ifdef HAVE_CONFIG_H" >$@ && \
	echo "# include <config.h>" >>$@ && \
	echo "#endif" >>$@ && \
	cat $@.tmp >>$@ && \
	rm $@.tmp

%.ui.h: %.ui
	$(UIC3) $< -o $<.tmp && \
	echo "#include <kbanking/kbanking.h>" >$@ && \
	$(top_srcdir)/admin/insert_api.sh KBANKING_API < $<.tmp >>$@ && \
	rm $<.tmp

SUFFIXES = .ui .ui.h .ui.cpp .moc.cpp


built_sources: $(BUILT_SOURCES)

