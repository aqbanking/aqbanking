
AM_CFLAGS=-DBUILDING_AQFINTS @visibility_cflags@

INCLUDES = -I$(top_srcdir)/src/libs -I$(top_builddir)/src/libs $(gwenhywfar_includes) \
  -I$(srcdir)/../.. \
  -I$(srcdir)/.. \
  -I$(aqbanking_symlinkdir) $(all_includes) \
  -I$(top_srcdir)/src/libs/aqbanking


DEFS+=-DAH_XMLDATADIR=\"$(aqbanking_data_searchdir)/aqbanking/backends/aqhbci\"


noinst_LTLIBRARIES=libafmsgparser.la


typefiles=\
  element.xml \
  segment.xml



typedatadir=$(datadir)/aqbanking/typemaker2/c
dist_typedata_DATA=\
  aqfints_element.tm2 aqfints_element_tree2.tm2



built_sources=  \
  element.c \
  segment.c


build_headers_priv= \
  element_p.h \
  segment_p.h


build_headers_pub= \
  element.h \
  segment.h


build_headers=$(build_headers_priv) $(build_headers_pub)



noinst_HEADERS= $(build_headers)


libafmsgparser_la_SOURCES= $(built_sources) \
  parser.c \
  parser_xml.c \
  parser_normalize.c \
  parser_dump.c \
  parser_hbci.c \
  parser_db.c



noinst_PROGRAMS = libtest


libtest_SOURCES = libtest.c
libtest_LDADD = libafmsgparser.la $(gwenhywfar_libs)




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
	for f in $(libafmsgparser_la_SOURCES) $(EXTRA_DIST); do \
	  echo $(subdir)/$$f >>$(top_srcdir)/i18nsources; \
	done
	for d in $(SUBDIRS); do \
	  $(MAKE) -C $$d sources; \
	done


types: $(typefiles)
	$(typemaker2_exe) -I $(top_srcdir)/src/libs/aqbanking/typemaker2/c -I $(srcdir) --destfolder="$(srcdir)" $(typefiles)



cppcheck:
	for f in $(libafmsgparser_la_SOURCES); do \
	  cppcheck --force $$f ; \
	done
	for d in $(SUBDIRS); do \
	  $(MAKE) -C $$d cppcheck; \
	done


