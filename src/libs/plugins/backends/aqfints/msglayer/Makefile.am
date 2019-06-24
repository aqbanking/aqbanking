
AM_CFLAGS=-DBUILDING_AQHBCI @visibility_cflags@

INCLUDES = -I$(top_srcdir)/src/libs -I$(top_builddir)/src/libs $(gwenhywfar_includes) \
  -I$(srcdir)/../.. \
  -I$(aqbanking_symlinkdir) $(all_includes) \
  -I$(top_srcdir)/src/libs/aqbanking


EXTRA_DIST=

DEFS+=-DAH_XMLDATADIR=\"$(aqbanking_data_searchdir)/aqbanking/backends/aqhbci\"


noinst_LTLIBRARIES=libaqfintsmsg.la


typefiles=\
  parser_element.xml


typedatadir=$(datadir)/aqbanking/typemaker2/c
dist_typedata_DATA=\
  aqfints_parser_element.tm2 aqfints_parser_element_tree.tm2



built_sources=  \
  parser_element.c


build_headers_priv= \
  parser_element_p.h


build_headers_pub= \
  parser_element.h


build_headers=$(build_headers_priv) $(build_headers_pub)



noinst_HEADERS= $(build_headers)


libaqfintsmsg_la_SOURCES= $(built_sources)



sources:
	for f in $(libaqfintsmsg_la_SOURCES) $(EXTRA_DIST); do \
	  echo $(subdir)/$$f >>$(top_srcdir)/i18nsources; \
	done
	for d in $(SUBDIRS); do \
	  $(MAKE) -C $$d sources; \
	done


types: $(typefiles)
	$(typemaker2_exe) -I $(top_srcdir)/src/libs/aqbanking/typemaker2/c -I $(srcdir) --destfolder="$(srcdir)" $(typefiles)



cppcheck:
	for f in $(libaqfintsmsg_la_SOURCES); do \
	  cppcheck --force $$f ; \
	done
	for d in $(SUBDIRS); do \
	  $(MAKE) -C $$d cppcheck; \
	done


