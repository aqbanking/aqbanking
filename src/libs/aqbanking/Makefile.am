SUBDIRS=jobs types

INCLUDES=@all_includes@

lib_LTLIBRARIES=libaqbanking.la

AM_CFLAGS=-DAQBANKING_PLUGINS=\"@aqbanking_plugins@\"

noinst_HEADERS=\
 account_l.h \
 account_p.h \
 banking_l.h \
 banking_p.h \
 job_l.h \
 job_p.h \
 provider_l.h \
 provider_p.h


iheaderdir=${includedir}/aqbanking
iheader_HEADERS=\
 account.h \
 banking.h \
 error.h \
 job.h \
 provider.h

libaqbanking_la_SOURCES=\
 account.c \
 banking.c \
 job.c \
 provider.c

libaqbanking_la_LIBADD=@gwenhywfar_libs@ \
 jobs/libjobs.la \
 types/libtypes.la

