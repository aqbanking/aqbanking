INCLUDES = \
  -I$(aqbanking_symlinkdir) \
  -I$(srcdir)/../widgets -I$(srcdir)/../dialogs -I$(srcdir)/.. \
  -I../widgets -I../dialogs -I.. \
  $(GWENGUI_QT3_CFLAGS) \
  $(all_includes)

DEFS += -DBUILDING_QBANKING
AM_CPPFLAGS= @visibility_cflags@

noinst_LTLIBRARIES=libviews.la

iheaderdir=@aqbanking_headerdir_am@/qbanking
iheader_HEADERS=\
 qbbankselector.h


nodist_iheader_HEADERS= \
 qbbankselector.ui.h

libviews_la_SOURCES=\
 qbbankselector.cpp

MOC_FILES=\
 qbbankselector.ui.moc qbbankselector.moc

nodist_libviews_la_SOURCES=\
 qbbankselector.ui.cpp

UI_FILES = qbbankselector.ui


EXTRA_DIST = $(UI_FILES)

# Make sure the generated source files are generated before any
# further compiling.
BUILT_SOURCES = $(MOC_FILES) $(nodist_libviews_la_SOURCES) $(nodist_iheader_HEADERS)
CLEANFILES = $(BUILT_SOURCES)

sources:
	for f in $(libviews_la_SOURCES) $(foreach uifile,$(UI_FILES),$(uifile).cpp); do \
	  echo $(subdir)/$$f >>$(top_srcdir)/qtsources; \
	done


%.moc: %.h
	$(qt3_moc) -o $@ $<

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

SUFFIXES = .ui .ui.h .ui.cpp .moc

# For qt3 compatibility of qt4
DEFS += -DQT3_SUPPORT


built_sources: $(BUILT_SOURCES)

