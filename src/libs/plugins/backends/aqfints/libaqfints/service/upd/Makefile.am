
AM_CFLAGS=@visibility_cflags@

AM_CPPFLAGS = -I$(top_srcdir)/src/libs -I$(top_builddir)/src/libs $(gwenhywfar_includes) $(aqbanking_includes) \
  -I$(srcdir)/../../.. \
  -I$(srcdir)/../.. \
  -I$(srcdir)/..


noinst_LTLIBRARIES=libafmsgupd.la


typefiles= \
  updjob.xml \
  accountdata.xml \
  userdata.xml



fintsdatadir=$(aqfints_datadir)/fints
fintsdata_DATA=\
  upd.fints





typedatadir=$(datadir)/aqbanking/typemaker2/c
dist_typedata_DATA=



built_sources=\
  updjob.c \
  accountdata.c \
  userdata.c


build_headers_priv=\
  updjob_p.h \
  accountdata_p.h \
  userdata_p.h


build_headers_pub=\
  updjob.h \
  accountdata.h \
  userdata.h



build_headers=$(build_headers_priv) $(build_headers_pub)



noinst_HEADERS= $(build_headers) \
  upd_read.h \
  upd_write.h


libafmsgupd_la_SOURCES= $(built_sources) \
  upd_read.c \
  upd_write.c





EXTRA_DIST=$(typefiles) $(built_sources) $(build_headers) $(fintsdata_DATA)



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
	for f in $(libafmsgupd_la_SOURCES) $(EXTRA_DIST); do \
	  echo $(subdir)/$$f >>$(top_srcdir)/i18nsources; \
	done
	for d in $(SUBDIRS); do \
	  $(MAKE) -C $$d sources; \
	done


typefiles: $(typefiles)
	for d in $(SUBDIRS); do \
	  $(MAKE) -C $$d typefiles; \
	done
	$(typemaker2_exe) -I $(top_srcdir)/src/libs/aqbanking/typemaker2/c -I $(aqbanking_dir)/share/aqbanking/aqbanking/typemaker2/c -I $(gwenhywfar_dir)/share/gwenhywfar/typemaker2/c -I $(srcdir) --destfolder="$(srcdir)" $(typefiles)

typedefs: $(typefiles)
	for d in $(SUBDIRS); do \
	  $(MAKE) -C $$d typedefs; \
	done
	$(typemaker2_exe) --defs-only -I $(top_srcdir)/src/libs/aqbanking/typemaker2/c -I $(aqbanking_dir)/share/aqbanking/aqbanking/typemaker2/c -I $(gwenhywfar_dir)/share/gwenhywfar/typemaker2/c -I $(srcdir) --destfolder="$(srcdir)" $(typefiles)



cppcheck:
	for f in $(libafmsgupd_la_SOURCES); do \
	  cppcheck --force $$f ; \
	done
	for d in $(SUBDIRS); do \
	  $(MAKE) -C $$d cppcheck; \
	done


