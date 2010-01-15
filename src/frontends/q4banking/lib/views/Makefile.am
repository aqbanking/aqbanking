INCLUDES = \
  -I$(top_builddir)/headers \
  -I$(srcdir)/../widgets -I$(srcdir)/../dialogs -I$(srcdir)/.. \
  -I../widgets -I../dialogs -I.. $(qt4_includes) $(gwenhywfar_includes)


DEFS += -DBUILDING_Q4BANKING
AM_CPPFLAGS= @visibility_cflags@

noinst_LTLIBRARIES=libviews.la

iheaderdir=${includedir}/q4banking
iheader_HEADERS=\
 qbbankselector.h


nodist_iheader_HEADERS= \
 qbbankselector.ui.h

libviews_la_SOURCES=\
 qbbankselector.cpp

MOC_FILES=\
 qbbankselector.ui.moc qbbankselector.moc

nodist_libviews_la_SOURCES=

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
	$(qt4_moc) -o $@ $<

%.ui.h: %.ui
	$(qt4_uic) $< -o $@

SUFFIXES = .ui .ui.h .moc

# For qt4 compatibility of qt4
DEFS += -DQT3_SUPPORT


built_sources: $(BUILT_SOURCES)

