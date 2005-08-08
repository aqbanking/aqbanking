INCLUDES=-I$(top_builddir)/headers -I. @all_includes@

#EXTRA_DIST=

noinst_LTLIBRARIES=libprg.la

iheaderdir=${includedir}/kbanking
iheader_HEADERS=\
 banking.h \
 banking_p.h

libprg_la_SOURCES=\
 banking.cpp



sources:
	for f in $(libprg_la_SOURCES) $(UI_FILES); do \
	  echo $(subdir)/$$f >>../i18nsources; \
	done
