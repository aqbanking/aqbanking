SUBDIRS=profiles data

AM_CPPFLAGS = -I$(top_srcdir)/src/libs \
  -I$(top_builddir)/src/libs \
  $(gwenhywfar_includes)

AM_CFLAGS=-DBUILDING_AQBANKING @visibility_cflags@

TM2_INCLUDES=\
  -I $(top_srcdir)/src/libs/aqbanking/typemaker2/c \
  -I $(gwenhywfar_dir)/share/gwenhywfar/typemaker2/c  \
  -I $(srcdir) \
  -I $(top_srcdir)/src/libs/aqbanking/types



typefiles=\
  paymentgroup.xml


built_sources=  \
  paymentgroup.c


build_headers_priv= \
  paymentgroup_p.h \
  paymentgroup.h


build_headers=$(build_headers_priv) $(build_headers_pub)



imexporterplugindir = $(aqbanking_plugindir)/imexporters
noinst_LTLIBRARIES=libabimexporters_xml.la
imexporterplugin_DATA=xml.xml

libabimexporters_xml_la_SOURCES=$(built_sources) \
  xml.c \
  xml_sepa_exp.c \
  xml_sepa_imp.c


noinst_HEADERS=$(build_headers_priv) \
  xml_p.h \
  xml.h \
  xml_sepa_exp.h \
  xml_sepa_imp.h





EXTRA_DIST=$(typefiles) $(built_sources) $(build_headers) README



typefiles: $(typefiles)
	$(typemaker2_exe) $(TM2_INCLUDES) --destfolder="$(srcdir)" $(typefiles)

typedefs: $(typefiles)
	$(typemaker2_exe) --defs-only  $(TM2_INCLUDES) --destfolder="$(srcdir)" $(typefiles)



sources:
	for f in $(libabimexporters_xml_la_SOURCES) $(built_sources) $(build_headers); do \
	  echo $(subdir)/$$f >>$(top_srcdir)/i18nsources; \
	done
	for f in $(imexporterplugin_DATA); do \
	  echo $(subdir)/$$f >>$(top_srcdir)/pdsources; \
	done

cppcheck:
	for f in $(libabimexporters_xml_la_SOURCES); do \
	  cppcheck --force $$f ; \
	done


