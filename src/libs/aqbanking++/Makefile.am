INCLUDES=-I${top_srcdir} @all_includes@

lib_LTLIBRARIES=libaqbankingpp.la

AM_CFLAGS=-DAQBANKING_PLUGINS=\"@aqbanking_plugins@\"

noinst_HEADERS=\
 banking_p.h \
 provider_p.h


iheaderdir=${includedir}/aqbanking++
iheader_HEADERS=\
 banking.h \
 provider.h

libaqbankingpp_la_SOURCES=\
 banking.cpp \
 provider.cpp

libaqbankingpp_la_LIBADD=@gwenhywfar_libs@ -L../aqbanking -laqbanking

