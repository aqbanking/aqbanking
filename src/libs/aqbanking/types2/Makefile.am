#SUBDIRS=typemaker2


INCLUDES = -I$(srcdir)/.. \
  -I$(aqbanking_symlinkdir) $(gwenhywfar_includes)

AM_CFLAGS=-DBUILDING_AQBANKING @visibility_cflags@


noinst_LTLIBRARIES=libabtypes2.la



typefiles=\
  transaction.xml \
  transactionlimits.xml \
  bankinfoservice.xml \
  bankinfo.xml \
  document.xml \
  message.xml \
  balance.xml \
  imexporter_accountinfo.xml \
  imexporter_context.xml \
  account_spec.xml


typedatadir=$(datadir)/aqbanking/typemaker2/c
dist_typedata_DATA=\
  ab_account_spec.tm2 ab_account_spec_list.tm2 ab_account_spec_list2.tm2 \
  ab_balance.tm2 ab_balance_list.tm2 \
  ab_bankinfo.tm2 ab_bankinfo_list.tm2 ab_bankinfo_list2.tm2 \
  ab_bankinfo_service.tm2 ab_bankinfo_service_list.tm2 ab_bankinfo_service_list2.tm2 \
  ab_document.tm2 ab_document_list.tm2 ab_document_list2.tm2 \
  ab_imexporter_accountinfo.tm2 ab_imexporter_accountinfo_list.tm2 ab_imexporter_accountinfo_list2.tm2 \
  ab_imexporter_context.tm2 ab_imexporter_context_list.tm2 ab_imexporter_context_list2.tm2 \
  ab_message.tm2 ab_message_list.tm2 ab_message_list2.tm2 \
  ab_security.tm2 ab_security_list.tm2 ab_security_list2.tm2 \
  ab_transaction.tm2 ab_transaction_list.tm2 ab_transaction_list2.tm2 \
  ab_transaction_limits.tm2 ab_transaction_limits_list.tm2 ab_transaction_limits_list2.tm2


built_sources=  \
  transaction.c \
  transactionlimits.c \
  bankinfoservice.c \
  bankinfo.c \
  document.c \
  message.c \
  security.c \
  balance.c \
  imexporter_accountinfo.c \
  imexporter_context.c \
  account_spec.c


build_headers_priv= \
  transaction_p.h   \
  transactionlimits_p.h \
  bankinfoservice_p.h \
  bankinfo_p.h \
  document_p.h \
  message_p.h \
  security_p.h \
  balance_p.h \
  imexporter_accountinfo_p.h \
  imexporter_context_p.h \
  account_spec_p.h


build_headers_pub= \
  transaction.h    \
  transactionlimits.h \
  bankinfoservice.h \
  bankinfo.h \
  document.h \
  message.h \
  security.h \
  balance.h \
  imexporter_accountinfo.h \
  imexporter_context.h \
  account_spec.h



build_headers=$(build_headers_priv) $(build_headers_pub)


libabtypes2_la_SOURCES=$(built_sources)


iheaderdir=@aqbanking_headerdir_am@/aqbanking
iheader_HEADERS=$(build_headers_pub)


noinst_HEADERS=$(build_headers_priv)



EXTRA_DIST=$(typefiles) $(built_sources) $(build_headers)



built_sources: $(built_sources)
	list='$(SUBDIRS)'; for d in $$list; do \
	  $(MAKE) -C $$d built_sources; \
	done ;

clean_build_files:
	rm -f $(built_sources) $(build_headers)

format_build_files:
	for f in $(built_sources) $(build_headers); do \
	  astyle --style=linux -s2 --min-conditional-indent=0 --indent-labels --pad-oper --suffix=none $$f; \
	done

sources:
	for f in $(libabtypes2_la_SOURCES) $(EXTRA_DIST); do \
	  echo $(subdir)/$$f >>$(top_srcdir)/i18nsources; \
	done
	for d in $(SUBDIRS); do \
	  $(MAKE) -C $$d sources; \
	done




%.c: %.xml
	$(typemaker2_exe) -I $(top_srcdir)/src/libs/aqbanking/typemaker2/c -I $(srcdir) -I $(builddir) --destfolder="$(builddir)" -D AQBANKING_API  $<

%.h: %.xml
	$(typemaker2_exe) -I $(top_srcdir)/src/libs/aqbanking/typemaker2/c -I $(srcdir) -I $(builddir) --destfolder="$(builddir)" -D AQBANKING_API  $<


SUFFIXES = .xml .c


types: $(typefiles)
	$(typemaker2_exe) -I $(top_srcdir)/src/libs/aqbanking/typemaker2/c -I $(srcdir) --destfolder="$(srcdir)" -D AQBANKING_API $(typefiles)

typedefs: $(typefiles)
	$(typemaker2_exe) --defs-only -I $(top_srcdir)/src/libs/aqbanking/typemaker2/c -I $(srcdir) --destfolder="$(srcdir)" -D AQBANKING_API $(typefiles)



cppcheck:
	for f in $(libabtypes2_la_SOURCES); do \
	  cppcheck --force $$f ; \
	done


