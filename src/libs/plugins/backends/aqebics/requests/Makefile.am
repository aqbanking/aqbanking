

INCLUDES = -I$(top_srcdir)/src/libs -I$(top_builddir)/src/libs $(gwenhywfar_includes) -I$(srcdir)/../.. $(AQEBICS_CFLAGS)

DEFS += -DLOCALEDIR=\"$(localedir)\"
AM_CFLAGS=-DBUILDING_AQEBICS @visibility_cflags@

#typefiles=context.xml

noinst_LTLIBRARIES=libaqebics_requests.la

extra_sources=

EXTRA_DIST=$(typefiles) $(extra_sources)


noinst_HEADERS=\
  r_download_l.h \
  r_hia_l.h \
  r_hkd_htd_l.h \
  r_hpb_l.h \
  r_hpd_l.h \
  r_ini_l.h \
  r_pub_l.h \
  r_unsecured_l.h \
  r_upload_l.h


#iheaderdir=${includedir}/aqebics/client
#iheader_HEADERS=


libaqebics_requests_la_SOURCES=\
  r_download.c \
  r_download_h002.c \
  r_download_h003.c \
  r_hia.c \
  r_hia_h002.c \
  r_hia_h003.c \
  r_hia_h004.c \
  r_hkd_htd.c \
  r_hpb.c \
  r_hpb_h002.c \
  r_hpb_h003.c \
  r_hpb_h004.c \
  r_hpd.c \
  r_ini.c \
  r_ini_h002.c \
  r_ini_h003.c \
  r_ini_h004.c \
  r_pub.c \
  r_pub_h002.c \
  r_pub_h003.c \
  r_unsecured.c \
  r_unsecured_h004.c \
  r_upload.c \
  r_upload_h002.c \
  r_upload_h003.c



#types: $(typefiles)
#	typemaker -d static $(typefiles)

types: $(typefiles)
	typemaker $(typefiles)

sources:
	for d in $(SUBDIRS); do \
	  $(MAKE) -C $$d sources; \
	done
	for f in $(libaqebics_requests_la_SOURCES) $(EXTRA_SOURCES); do \
	  echo $(subdir)/$$f >>$(top_srcdir)/i18nsources; \
	done


cppcheck:
	for f in $(libaqebics_requests_la_SOURCES); do \
	  cppcheck --force $$f ; \
	done
	for d in $(SUBDIRS); do \
	  $(MAKE) -C $$d cppcheck; \
	done

