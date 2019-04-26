INCLUDES = -I$(srcdir)/../.. -I$(srcdir)/.. -I$(top_builddir)/src/libs $(gwenhywfar_includes)

AM_CFLAGS=-DBUILDING_AQBANKING @visibility_cflags@

noinst_LTLIBRARIES=libabdialogs.la

#iheaderdir=@aqbanking_headerdir_am@/aqbanking
#iheader_HEADERS=

noinst_HEADERS=\
  dlg_importer.h \
  dlg_selectbankinfo.h \
  dlg_setup.h \
  dlg_selectbackend.h \
  dlg_editaccount.h \
  dlg_edituser.h \
  dlg_usertype_page_be.h \
  dlg_setup_newuser.h \
  dlg_importer_p.h \
  dlg_selectbankinfo_p.h \
  dlg_setup_p.h \
  dlg_selectbackend_p.h \
  dlg_editaccount_p.h \
  dlg_edituser_p.h \
  dlg_usertype_page_p.h \
  dlg_setup_newuser_p.h \
  dlg_usertype_pagedefault_l.h \
  dlg_usertype_pagedefault_p.h \
  dlg_newaccount_be.h \
  dlg_newaccount_p.h \
  dlg_newuser_be.h \
  dlg_newuser_p.h

libabdialogs_la_SOURCES=\
  dlg_importer.c \
  dlg_selectbankinfo.c \
  dlg_setup.c \
  dlg_selectbackend.c \
  dlg_editaccount.c \
  dlg_edituser.c \
  dlg_usertype_page.c \
  dlg_setup_newuser.c \
  dlg_usertype_pagedefault.c \
  dlg_newaccount.c \
  dlg_newuser.c


dlgdatadir=$(aqbanking_pkgdatadir)/dialogs
dlgdata_DATA=\
  dlg_importer.dlg \
  dlg_selectbankinfo.dlg \
  dlg_setup.dlg \
  dlg_selectbackend.dlg \
  dlg_editaccount.dlg \
  dlg_edituser.dlg \
  dlg_setup_newuser.dlg \
  dlg_usertype_pagedefault.dlg




EXTRA_DIST=$(typefiles) $(dlgdata_DATA)


sources:
	for f in $(libabdialogs_la_SOURCES); do \
	  echo $(subdir)/$$f >>$(top_srcdir)/i18nsources; \
	done
	for f in $(dlgdata_DATA); do \
	  echo $(subdir)/$$f >>$(top_srcdir)/dlgsources; \
	done
	for d in $(SUBDIRS); do \
	  $(MAKE) -C $$d sources; \
	done


types:

typedefs:

built_sources:


cppcheck:
	for f in $(libabdialogs_la_SOURCES); do \
	  cppcheck --force $$f ; \
	done

