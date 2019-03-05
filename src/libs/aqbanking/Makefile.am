SUBDIRS=types gui backendsupport typemaker2 dialogs

INCLUDES = \
  $(gwenhywfar_includes) \
  -I$(srcdir)/../

noinst_LTLIBRARIES=libaqbanking_base.la

AM_CFLAGS=-DBUILDING_AQBANKING @visibility_cflags@

EXTRA_DIST=\
 banking_account.c \
 banking_accspec.c \
 banking_bankinfo.c \
 banking_cfg.c \
 banking_dialogs.c \
 banking_imex.c \
 banking_init.c \
 banking_online.c \
 banking_transaction.c \
 banking_update.c \
 banking_user.c \
 banking_compat.c \
 system.h.in \
 version.h.in




# We define those here because within the configure script the variables 
# $(datadir) et al are not available.
DEFS += \
 -DEXEEXT=\"$(EXEEXT)\" \
 -DLOCALEDIR=\"@aqbanking_locale_searchdir@\" \
 -DAQBANKING_PLUGINS=\"$(aqbanking_plugin_searchdir)\" \
 -DAQBANKING_SYSCONF_DIR=\"$(aqbanking_cfg_searchdir)\" \
 -DAQBANKING_DATA_DIR=\"$(aqbanking_data_searchdir)\" \
 -DCOMPILE_DATETIME=\"@DATETIME@\"

noinst_HEADERS=\
 banking_be.h \
 banking_l.h \
 banking_p.h \
 i18n_l.h


iheaderdir=@aqbanking_headerdir_am@/aqbanking
iheader_HEADERS=\
 account_type.h \
 banking.h \
 banking_dialogs.h \
 banking_imex.h \
 banking_bankinfo.h \
 banking_online.h \
 banking_transaction.h \
 error.h \
 system.h \
 version.h


libaqbanking_base_la_SOURCES=\
 account_type.c \
 banking.c


libaqbanking_base_la_LIBADD= \
 backendsupport/libabbesupport.la \
 types/libabtypes.la \
 gui/libabgui.la \
 dialogs/libabdialogs.la


sources:
	for f in $(libaqbanking_base_la_SOURCES) $(EXTRA_DIST); do \
	  echo $(subdir)/$$f >>$(top_srcdir)/i18nsources; \
	done
	for d in $(SUBDIRS); do \
	  $(MAKE) -C $$d sources; \
	done

built_sources:
	for d in $(SUBDIRS); do \
	  $(MAKE) -C $$d built_sources; \
	done ;


types:
	for d in $(SUBDIRS); do \
	  $(MAKE) -C $$d types; \
	done

typedefs:
	for d in $(SUBDIRS); do \
	  $(MAKE) -C $$d typedefs; \
	done



cppcheck:
	for f in $(libaqbanking_base_la_SOURCES); do \
	  cppcheck --force $$f ; \
	done
	for d in $(SUBDIRS); do \
	  $(MAKE) -C $$d cppcheck; \
	done


