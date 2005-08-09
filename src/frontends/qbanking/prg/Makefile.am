INCLUDES=-I$(top_builddir)/headers -I. @all_includes@

#EXTRA_DIST=

noinst_LTLIBRARIES=

iheaderdir=${includedir}/qbanking
iheader_HEADERS=

#libprg_la_SOURCES=



sources:
	for f in $(libprg_la_SOURCES) $(UI_FILES); do \
	  echo $(subdir)/$$f >>$(top_srcdir)/qtsources; \
	done
