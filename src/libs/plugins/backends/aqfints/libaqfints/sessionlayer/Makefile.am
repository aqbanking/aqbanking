SUBDIRS=pintan hbci


AM_CFLAGS=-DBUILDING_AQBANKING @visibility_cflags@

AM_CPPFLAGS = -I$(top_srcdir)/src/libs -I$(top_builddir)/src/libs $(gwenhywfar_includes) $(aqbanking_includes) \
  -I$(srcdir)/../.. \
  -I$(srcdir)/..


noinst_LTLIBRARIES=libaqfintssession.la


typefiles=\
  cryptparams.xml




typedatadir=$(datadir)/aqbanking/typemaker2/c
dist_typedata_DATA=



built_sources=\
  cryptparams.c


build_headers_priv=\
  cryptparams_p.h



build_headers_pub=\
  cryptparams.h




build_headers=$(build_headers_priv) $(build_headers_pub)



noinst_HEADERS= $(build_headers) \
  s_decode.h \
  s_decrypt.h \
  s_decrypt_rdh.h \
  s_message.h \
  s_verify.h \
  s_verify_rdh.h \
  s_sign.h \
  s_encrypt.h \
  s_encode.h



libaqfintssession_la_SOURCES=$(built_sources) \
  session.c \
  s_getanonbpd.c \
  s_message.c \
  s_log.c \
  s_decode.c \
  s_decrypt.c \
  s_decrypt_rdh.c \
  s_verify.c \
  s_verify_rdh.c \
  s_sign.c \
  s_encrypt.c \
  s_encode.c



libaqfintssession_la_LIBADD=\
  pintan/libaqfintssession_pin.la \
  hbci/libaqfintssession_hbci.la



EXTRA_DIST=$(typefiles) $(built_sources) $(build_headers)



built_sources: $(built_sources)
	list='$(SUBDIRS)'; for d in $$list; do \
	  $(MAKE) -C $$d built_sources; \
	done ;

clean_build_files:
	rm -f $(built_sources) $(build_headers)

format_built_files:
	for f in $(built_sources) $(build_headers); do \
	  $(ASTYLE) --style=stroustrup -s2 --min-conditional-indent=0 --indent-labels --max-continuation-indent=80 --pad-comma --pad-header --unpad-paren --align-pointer=name --break-closing-braces --break-one-line-headers --attach-return-type --convert-tabs --max-code-length=120 --break-after-logical --suffix=none $$f; \
	done


sources:
	for f in $(libaqfintssession_la_SOURCES) $(EXTRA_DIST); do \
	  echo $(subdir)/$$f >>$(top_srcdir)/i18nsources; \
	done
	for d in $(SUBDIRS); do \
	  $(MAKE) -C $$d sources; \
	done


typefiles: $(typefiles)
	for d in $(SUBDIRS); do \
	  $(MAKE) -C $$d typefiles; \
	done
	$(typemaker2_exe) -I $(top_srcdir)/src/libs/aqbanking/typemaker2/c -I $(gwenhywfar_dir)/share/gwenhywfar/typemaker2/c -I $(srcdir) --destfolder="$(srcdir)" $(typefiles)

typedefs: $(typefiles)
	for d in $(SUBDIRS); do \
	  $(MAKE) -C $$d typedefs; \
	done
	$(typemaker2_exe) --defs-only -I $(top_srcdir)/src/libs/aqbanking/typemaker2/c -I $(gwenhywfar_dir)/share/gwenhywfar/typemaker2/c -I $(srcdir) --destfolder="$(srcdir)" $(typefiles)


cppcheck:
	for f in $(libaqfintssession_la_SOURCES); do \
	  cppcheck --force $$f ; \
	done
	for d in $(SUBDIRS); do \
	  $(MAKE) -C $$d cppcheck; \
	done


