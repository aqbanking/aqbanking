INCLUDES=-I${top_srcdir} @all_includes@

lib_LTLIBRARIES=libaqbankingpp.la

AM_CFLAGS=-DAQBANKING_PLUGINS=\"@aqbanking_plugindir@\"

noinst_HEADERS=\
 banking_p.h


iheaderdir=${includedir}/aqbanking++
iheader_HEADERS=\
 banking.h

libaqbankingpp_la_SOURCES=\
 banking.cpp

libaqbankingpp_la_LIBADD=-L../aqbanking -laqbanking @gwenhywfar_libs@

sources:
	for f in $(libaqbankingpp_la_SOURCES); do \
	  echo $(subdir)/$$f >>$(top_srcdir)/i18nsources; \
	done
