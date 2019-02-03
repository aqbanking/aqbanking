INCLUDES=-I$(aqbanking_symlinkdir) @all_includes@  -I.. -I$(srcdir)/..

AM_CFLAGS=-DBUILDING_AQBANKING @visibility_cflags@


typefiles= \
  ab_accountqueue.xml \
  ab_userqueue.xml \
  ab_providerqueue.xml \
  ab_jobqueue.xml \
  ab_queue.xml \
  account.xml \
  user.xml


typedatadir=$(aqbanking_pkgdatadir)/typemaker2/c
dist_typedata_DATA=\
  ab_account.tm2 ab_account_list.tm2 ab_account_list2.tm2 \
  ab_accountqueue.tm2 ab_accountqueue_list.tm2 \
  ab_jobqueue.tm2 ab_jobqueue_list.tm2 \
  ab_providerqueue.tm2 ab_providerqueue_list.tm2 \
  ab_queue.tm2 \
  ab_user.tm2 ab_user_list.tm2 ab_user_list2.tm2 \
  ab_userqueue.tm2 ab_userqueue_list.tm2


# order is important here
typedata_sources= \
  ab_jobqueue.c \
  ab_accountqueue.c \
  ab_userqueue.c \
  ab_providerqueue.c \
  ab_queue.c \
  account.c \
  user.c

built_headers_priv= \
  ab_accountqueue_p.h \
  ab_userqueue_p.h \
  ab_providerqueue_p.h \
  ab_jobqueue_p.h \
  ab_queue_p.h \
  account_p.h \
  user_p.h

built_headers_pub= \
  ab_accountqueue.h \
  ab_userqueue.h \
  ab_providerqueue.h \
  ab_jobqueue.h \
  ab_queue.h \
  account.h \
  user.h



built_headers=$(built_headers_priv) $(built_headers_pub)


#iheaderdir=@aqbanking_headerdir_am@/aqbanking
#iheader_HEADERS=$(built_headers_pub)


noinst_HEADERS= $(built_headers_pub)\
  siotlsext.h \
  dlg_newaccount_be.h \
  dlg_newuser_be.h \
  httpsession.h \
  msgengine.h \
  provider.h \
  provider_be.h \
  bankinfoplugin.h \
  bankinfoplugin_be.h \
  siotlsext_p.h \
  dlg_newaccount_p.h \
  dlg_newuser_p.h \
  httpsession_p.h \
  msgengine_p.h \
  provider_l.h \
  provider_p.h \
  bankinfoplugin_l.h \
  bankinfoplugin_p.h \
  imexporter_be.h \
  imexporter_l.h \
  imexporter_p.h \
  imexporter.h


noinst_LTLIBRARIES=libabbesupport.la
libabbesupport_la_SOURCES=$(typedata_sources) \
  siotlsext.c \
  dlg_newaccount.c \
  dlg_newuser.c \
  httpsession.c \
  msgengine.c \
  provider.c \
  bankinfoplugin.c \
  imexporter.c


extra_sources=\
  provider_account.c \
  provider_accspec.c \
  provider_user.c \
  provider_queues.c



EXTRA_DIST=$(typefiles) $(typedata_sources) $(built_headers) $(extra_sources)



MAINTAINERCLEANFILES = $(typedata_sources) $(built_headers) $(dist_typedata_DATA)



built_sources: $(typedata_sources)

.xml.c:
	$(typemaker2_exe) -I $(top_srcdir)/src/libs/aqbanking/typemaker2/c -I $(srcdir) -I $(srcdir)/../types2 -I $(builddir) --destfolder="$(builddir)"  $<

.xml.h:
	$(typemaker2_exe) -I $(top_srcdir)/src/libs/aqbanking/typemaker2/c -I $(srcdir) -I $(srcdir)/../types2 -I $(builddir) --destfolder="$(builddir)"  $<

SUFFIXES = .xml .c


types: $(typefiles)
	$(typemaker2_exe) -I $(top_srcdir)/src/libs/aqbanking/typemaker2/c -I $(srcdir) -I $(srcdir)/../types2 --destfolder="$(srcdir)" $(typefiles)

typedefs: $(typefiles)
	$(typemaker2_exe) --defs-only -I $(top_srcdir)/src/libs/aqbanking/typemaker2/c -I $(srcdir) -I $(srcdir)/../types2 --destfolder="$(srcdir)" $(typefiles)


format_build_files:
	for f in $(typedata_sources) $(built_headers); do \
	  astyle --style=stroustrup -s2 --min-conditional-indent=0 --indent-labels --max-continuation-indent=80 --pad-comma --pad-header --unpad-paren --align-pointer=name --break-closing-braces --break-one-line-headers --attach-return-type --convert-tabs --max-code-length=120 --break-after-logical --suffix=none $$f; \
	done


sources: $(typedata_sources)
	for f in $(libabbesupport_la_SOURCES) $(extra_sources); do \
	  echo $(subdir)/$$f >>$(top_srcdir)/i18nsources; \
	done


cppcheck:
	for f in $(libabbesupport_la_SOURCES); do \
	  cppcheck --force $$f ; \
	done

