#SUBDIRS=

AM_CFLAGS=-DBUILDING_AQBANKING @visibility_cflags@

AM_CPPFLAGS = -I$(top_srcdir)/src/libs -I$(top_builddir)/src/libs $(gwenhywfar_includes) $(aqbanking_includes) \
  -I$(srcdir)/../../.. \
  -I$(srcdir)/../.. \
  -I$(srcdir)/.. \
  -I$(aqbanking_symlinkdir) $(all_includes)


noinst_LTLIBRARIES=libaqfintssession_pin.la


typefiles=



typedatadir=$(datadir)/aqbanking/typemaker2/c
dist_typedata_DATA=



built_sources=


build_headers_priv=



build_headers_pub=




build_headers=$(build_headers_priv) $(build_headers_pub)



noinst_HEADERS= $(build_headers) \
  s_pintan.h \
  s_pintan_p.h \
  s_decrypt_pintan.h \
  s_verify_pintan.h \
  s_sign_pintan.h \
  s_encrypt_pintan.h



libaqfintssession_pin_la_SOURCES=$(built_sources) \
  s_pintan.c \
  s_decrypt_pintan.c \
  s_verify_pintan.c \
  s_sign_pintan.c \
  s_encrypt_pintan.c




#libaqfintssession_pin_la_LIBADD=



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
	for f in $(libaqfintssession_pin_la_SOURCES) $(EXTRA_DIST); do \
	  echo $(subdir)/$$f >>$(top_srcdir)/i18nsources; \
	done
	for d in $(SUBDIRS); do \
	  $(MAKE) -C $$d sources; \
	done


typefiles:

typedefs:


cppcheck:
	for f in $(libaqfintssession_pin_la_SOURCES); do \
	  cppcheck --force $$f ; \
	done
	for d in $(SUBDIRS); do \
	  $(MAKE) -C $$d cppcheck; \
	done


