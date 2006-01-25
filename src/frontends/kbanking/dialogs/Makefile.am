INCLUDES=-I$(top_builddir)/headers -I$(srcdir)/../widgets -I$(srcdir)/.. -I$(srcdir)/../prg @all_includes@

noinst_LTLIBRARIES=libdialogs.la

#nodist_iheader_HEADERS=

iheaderdir=${includedir}/kbanking
iheader_HEADERS=\
 settings.h 

#nodist_libdialogs_la_SOURCES=


BUILT_SOURCES = $(nodist_libdialogs_la_SOURCES) $(nodist_iheader_HEADERS)
CLEANFILES = $(BUILT_SOURCES)

libdialogs_la_SOURCES=\
 settings.cpp


sources:
	for f in $(libdialogs_la_SOURCES) $(foreach uifile,$(UI_FILES),$(uifile).cpp); do \
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
	$(UIC3) $< -o $@.tmp && \
	echo "#include <kbanking/kbanking.h>" >$@ && \
	$(top_srcdir)/admin/insert_api.sh KBANKING_API < $@.tmp >>$@ && \
	rm $@.tmp

SUFFIXES = .ui .ui.h .ui.cpp .moc.cpp

built_sources: $(BUILT_SOURCES)

