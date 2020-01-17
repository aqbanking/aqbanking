
AM_CFLAGS=-DBUILDING_AQBANKING @visibility_cflags@

AM_CPPFLAGS = -I$(top_srcdir)/src/libs -I$(top_builddir)/src/libs -I$(srcdir)/../.. $(gwenhywfar_includes)


localedir = $(datadir)/locale
DEFS += -DLOCALEDIR=\"$(localedir)\"
noinst_LTLIBRARIES=libofxdcdialogs.la

dlgdatadir=$(aqbanking_pkgdatadir)/backends/aqofxconnect/dialogs
dlgdata_DATA=\
  dlg_edituser.dlg \
  dlg_newuser.dlg \
  dlg_ofx_special.dlg


noinst_HEADERS=\
  dlg_edituser_p.h dlg_edituser_l.h \
  dlg_newuser_p.h dlg_newuser_l.h \
  dlg_ofx_special_p.h dlg_ofx_special_l.h




#iheaderdir=@aqbanking_headerdir_am@/aqpaypal
#iheader_HEADERS=

libofxdcdialogs_la_SOURCES=\
  dlg_edituser.c \
  dlg_newuser.c \
  dlg_ofx_special.c



EXTRA_DIST= $(dlgdata_DATA) $(dlgimage_DATA)



sources:
	for f in $(libofxdcdialogs_la_SOURCES); do \
	  echo $(subdir)/$$f >>$(top_srcdir)/i18nsources; \
	done
	for d in $(SUBDIRS); do \
	  $(MAKE) -C $$d sources; \
	done


cppcheck:
	for f in $(libofxdcdialogs_la_SOURCES); do \
	  cppcheck --force $$f ; \
	done
	for d in $(SUBDIRS); do \
	  $(MAKE) -C $$d cppcheck; \
	done


built_sources:
	list='$(SUBDIRS)'; for d in $$list; do \
	  $(MAKE) -C $$d built_sources; \
	done ;


typefiles:

typedefs:



