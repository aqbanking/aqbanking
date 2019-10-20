AM_CPPFLAGS = -I$(top_srcdir)/src/libs -I$(top_builddir)/src/libs $(gwenhywfar_includes) -I$(srcdir)/../../ -I$(srcdir)/..

AM_CFLAGS=-DBUILDING_AQBANKING @visibility_cflags@


typefiles=\
 oh_institute_data.xml \
 oh_institute_spec.xml


#typedatadir=$(datadir)/aqbanking/aqofxconnect/typemaker2/c
#nodist_typedata_DATA=\
#  oh_institute_data.tm2 oh_institute_data_list.tm2 oh_institute_data_list2.tm2 \
#  oh_institute_spec.tm2 oh_institute_spec_list.tm2 oh_institute_spec_list2.tm2


dlgdatadir=$(aqbanking_pkgdatadir)/backends/aqofxconnect/dialogs
dlgdata_DATA=\
  dlg_getinst.dlg


build_sources= \
 oh_institute_data.c \
 oh_institute_spec.c

build_headers_priv= \
 oh_institute_data_p.h \
 oh_institute_spec_p.h


build_headers_pub= \
 oh_institute_data.h \
 oh_institute_spec.h

build_headers=$(build_headers_priv) $(build_headers_pub)


noinst_HEADERS=$(build_headers_priv) $(build_headers_pub) \
  ofxhome_p.h \
  dlg_getinst_p.h \
  oh_institute_data_fns.h \
  oh_institute_spec_fns.h \
  ofxhome.h \
  dlg_getinst.h



#iheaderdir=@aqbanking_headerdir_am@/aqofxconnect
#iheader_HEADERS=


noinst_LTLIBRARIES=libofxhome.la
libofxhome_la_SOURCES= $(build_sources) \
  ofxhome.c \
  dlg_getinst.c


EXTRA_DIST=\
 oh_institute_data_fns.c \
 oh_institute_spec_fns.c \
  $(typefiles) $(dlgdata_DATA) $(dlgimage_DATA) $(build_sources) $(build_headers)


built_sources: $(build_sources)



sources:
	for f in $(libofxhome_la_SOURCES); do \
	  echo $(subdir)/$$f >>$(top_srcdir)/i18nsources; \
	done
	for f in $(dlgdata_DATA); do \
	  echo $(subdir)/$$f >>$(top_srcdir)/dlgsources; \
	done
	for d in $(SUBDIRS); do \
	  $(MAKE) -C $$d sources; \
	done



#%.c: %.xml
#	$(gwenhywfar_bindir)/typemaker2 -I $(top_srcdir)/src/lib/typemaker2/c -I $(srcdir) --destfolder="$(srcdir)" -D AQOFXCONNECT_API  $<
#
#%.h: %.xml
#	$(gwenhywfar_bindir)/typemaker2 -I $(top_srcdir)/src/lib/typemaker2/c -I $(srcdir) --destfolder="$(srcdir)" -D AQOFXCONNECT_API  $<

#SUFFIXES = .xml .c


cppcheck:
	for f in $(libofxhome_la_SOURCES); do \
	  cppcheck --force $$f ; \
	done
	for d in $(SUBDIRS); do \
	  $(MAKE) -C $$d cppcheck; \
	done


typefiles: $(typefiles)
	$(typemaker2_exe) -I $(top_srcdir)/src/libs/aqbanking/typemaker2/c -I $(gwenhywfar_dir)/share/gwenhywfar/typemaker2/c -I $(srcdir) --destfolder="$(srcdir)" $(typefiles)

typedefs: $(typefiles)
	$(typemaker2_exe) --defs-only -I $(top_srcdir)/src/libs/aqbanking/typemaker2/c -I $(gwenhywfar_dir)/share/gwenhywfar/typemaker2/c -I $(srcdir) --destfolder="$(srcdir)" $(typefiles)

