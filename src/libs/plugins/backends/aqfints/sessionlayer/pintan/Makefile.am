#SUBDIRS=

AM_CFLAGS=-DBUILDING_AQBANKING @visibility_cflags@

AM_CPPFLAGS = -I$(top_srcdir)/src/libs -I$(top_builddir)/src/libs $(gwenhywfar_includes) \
  -I$(srcdir)/../../.. \
  -I$(srcdir)/../.. \
  -I$(srcdir)/.. \
  -I$(aqbanking_symlinkdir) $(all_includes) \
  -I$(top_srcdir)/src/libs/aqbanking


DEFS+=-DAH_XMLDATADIR=\"$(aqbanking_data_searchdir)/aqbanking/backends/aqfints\"


noinst_LTLIBRARIES=libaqfintssession_pin.la


typefiles=



typedatadir=$(datadir)/aqbanking/typemaker2/c
dist_typedata_DATA=



built_sources=


build_headers_priv=



build_headers_pub=




build_headers=$(build_headers_priv) $(build_headers_pub)



noinst_HEADERS= $(build_headers)



libaqfintssession_pin_la_SOURCES=$(built_sources) \
  s_pintan_sign.c



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
	  astyle --style=stroustrup -s2 --min-conditional-indent=0 --indent-labels --max-continuation-indent=80 --pad-comma --pad-header --unpad-paren --align-pointer=name --break-closing-braces --break-one-line-headers --attach-return-type --convert-tabs --max-code-length=120 --break-after-logical --suffix=none $$f; \
	done


sources:
	for f in $(libaqfintssession_pin_la_SOURCES) $(EXTRA_DIST); do \
	  echo $(subdir)/$$f >>$(top_srcdir)/i18nsources; \
	done
	for d in $(SUBDIRS); do \
	  $(MAKE) -C $$d sources; \
	done


types: $(typefiles)
	$(typemaker2_exe) -I $(top_srcdir)/src/libs/aqbanking/typemaker2/c -I $(srcdir) -I $(srcdir)../msglayer/parser -I $(srcdir)/../typemaker2 --destfolder="$(srcdir)" $(typefiles)



cppcheck:
	for f in $(libaqfintssession_pin_la_SOURCES); do \
	  cppcheck --force $$f ; \
	done
	for d in $(SUBDIRS); do \
	  $(MAKE) -C $$d cppcheck; \
	done


