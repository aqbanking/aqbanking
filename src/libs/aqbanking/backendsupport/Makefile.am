INCLUDES=-I$(aqbanking_symlinkdir) @all_includes@

AM_CFLAGS=-DBUILDING_AQBANKING @visibility_cflags@


typefiles= \
  ab_accountqueue.xml \
  ab_jobqueue.xml \
  ab_userqueue.xml \
  ab_queue.xml


typedatadir=$(aqbanking_pkgdatadir)/typemaker2/c
dist_typedata_DATA=\
  ab_accountqueue.tm2 \
  ab_accountqueue_list.tm2 \
  ab_userqueue.tm2 \
  ab_userqueue_list.tm2 \
  ab_queue.tm2 \
  ab_jobqueue.tm2 \
  ab_jobqueue_list.tm2


# order is important here
typedata_sources= \
  ab_jobqueue.c \
  ab_accountqueue.c \
  ab_userqueue.c \
  ab_queue.c

built_headers_priv= \
  ab_accountqueue_p.h \
  ab_jobqueue_p.h \
  ab_userqueue_p.h \
  ab_queue_p.h

built_headers_pub= \
  ab_accountqueue.h \
  ab_jobqueue.h \
  ab_userqueue.h \
  ab_queue.h



built_headers=$(built_headers_priv) $(built_headers_pub)


iheaderdir=@aqbanking_headerdir_am@/aqbanking
iheader_HEADERS=$(built_headers_pub)\
  ab_accountqueuefns.h \
  ab_userqueuefns.h \
  ab_queuefns.h

extra_sources=\
  ab_accountqueuefns.c \
  ab_userqueuefns.c \
  ab_queuefns.c


EXTRA_DIST=$(typefiles) $(typedata_sources) $(built_headers) $(extra_sources)



MAINTAINERCLEANFILES = $(typedata_sources) $(built_headers) $(dist_typedata_DATA)


noinst_LTLIBRARIES=libabbesupport.la
libabbesupport_la_SOURCES=$(typedata_sources)

built_sources: $(typedata_sources)

.xml.c:
	$(typemaker2_exe) -I $(top_srcdir)/src/libs/aqbanking/typemaker2/c -I $(srcdir) -I $(builddir) --destfolder="$(builddir)" -D AQBANKING_API  $<

.xml.h:
	$(typemaker2_exe) -I $(top_srcdir)/src/libs/aqbanking/typemaker2/c -I $(srcdir) -I $(builddir) --destfolder="$(builddir)" -D AQBANKING_API  $<

SUFFIXES = .xml .c


types: $(typefiles)
	$(typemaker2_exe) -I $(top_srcdir)/src/libs/aqbanking/typemaker2/c -I $(srcdir) --destfolder="$(srcdir)" -D AQBANKING_API $(typefiles)


sources: $(typedata_sources)
	for f in $(libabbesupport_la_SOURCES) $(extra_sources); do \
	  echo $(subdir)/$$f >>$(top_srcdir)/i18nsources; \
	done

