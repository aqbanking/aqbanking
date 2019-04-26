
INCLUDES = -I$(top_srcdir)/src/libs -I$(top_builddir)/src/libs $(gwenhywfar_includes) -I$(srcdir)/../.. $(AQEBICS_CFLAGS)

DEFS += -DLOCALEDIR=\"$(localedir)\" 

AM_CFLAGS=-DBUILDING_AQEBICS @visibility_cflags@

#typefiles=

noinst_LTLIBRARIES=libdialogs.la



noinst_HEADERS=\
 dlg_edituser_l.h dlg_edituser_p.h \
 dlg_newkeyfile_l.h dlg_newkeyfile_p.h \
 dlg_user_special_l.h dlg_user_special_p.h

libdialogs_la_SOURCES=\
 dlg_edituser.c \
 dlg_newkeyfile.c \
 dlg_user_special.c

dlgdatadir=$(datadir)/aqbanking/backends/aqebics/dialogs
dlgdata_DATA=\
 dlg_edituser.dlg \
 dlg_newkeyfile.dlg \
 dlg_user_special.dlg


EXTRA_DIST=$(typefiles) $(dlgdata_DATA)


types: $(typefiles)
	typemaker $(typefiles)

sources:
	for d in $(SUBDIRS); do \
	  $(MAKE) -C $$d sources; \
	done
	for f in $(dlgdata_DATA); do \
	  echo $(subdir)/$$f >>$(top_srcdir)/dlgsources; \
	done
	for f in $(libdialogs_la_SOURCES); do \
	  echo $(subdir)/$$f >>$(top_srcdir)/i18nsources; \
	done


cppcheck:
	for f in $(libdialogs_la_SOURCES); do \
	  cppcheck --force $$f ; \
	done
	for d in $(SUBDIRS); do \
	  $(MAKE) -C $$d cppcheck; \
	done

