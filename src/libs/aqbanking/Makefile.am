SUBDIRS=types jobs gui dialogs backendsupport typemaker2

INCLUDES = \
  -I$(aqbanking_symlinkdir) \
  -I$(srcdir)/types -I$(srcdir)/jobs $(gwenhywfar_includes)

lib_LTLIBRARIES=libaqbanking.la

AM_CFLAGS=-DBUILDING_AQBANKING @visibility_cflags@

EXTRA_DIST=\
 banking_account.c \
 banking_cfg.c \
 banking_deprec.c \
 banking_init.c \
 banking_online.c \
 banking_user.c \
 banking_imex.c


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
 account_l.h \
 account_p.h \
 banking_l.h \
 banking_p.h \
 bankinfoplugin_l.h \
 bankinfoplugin_p.h \
 country_l.h \
 country_p.h \
 i18n_l.h \
 imexporter_l.h \
 imexporter_p.h \
 job_l.h \
 job_p.h \
 provider_l.h \
 provider_p.h \
 msgengine_p.h \
 httpsession_p.h


iheaderdir=@aqbanking_headerdir_am@/aqbanking
iheader_HEADERS=\
 account.h \
 account_be.h \
 banking.h \
 banking_be.h \
 banking_cfg.h \
 banking_imex.h \
 banking_info.h \
 banking_ob.h \
 banking_simple.h \
 banking_virt.h \
 bankinfoplugin.h \
 bankinfoplugin_be.h \
 country.h \
 error.h \
 imexporter.h \
 imexporter_be.h \
 job.h \
 job_be.h \
 provider.h \
 provider_be.h\
 msgengine.h \
 httpsession.h


libaqbanking_la_SOURCES=\
 account.c \
 bankinfoplugin.c \
 banking.c \
 country.c \
 imexporter.c \
 job.c \
 provider.c \
 msgengine.c \
 httpsession.c


libaqbanking_la_LIBADD= $(gwenhywfar_libs) $(gmp_libs) $(i18n_libs) \
 backendsupport/libabbesupport.la \
 jobs/libjobs.la \
 types/libtypes.la \
 gui/libabgui.la \
 dialogs/libabdialogs.la

libaqbanking_la_LDFLAGS = -no-undefined \
  -version-info @AQBANKING_SO_CURRENT@:@AQBANKING_SO_REVISION@:@AQBANKING_SO_AGE@

sources:
	for f in $(libaqbanking_la_SOURCES) $(EXTRA_DIST); do \
	  echo $(subdir)/$$f >>$(top_srcdir)/i18nsources; \
	done
	for d in $(SUBDIRS); do \
	  $(MAKE) -C $$d sources; \
	done

built_sources:
	for d in  backendsupport ; do \
	  $(MAKE) -C $$d built_sources; \
	done ;

# Build and link a test program to verify the linker flags
noinst_PROGRAMS = testlib ab_value_test
testlib_SOURCES = testlib.c
testlib_LDADD = libaqbanking.la $(gwenhywfar_libs)

# Another test program to verify the parsing of decimal numbers
ab_value_test_SOURCES = ab-value-test.c
ab_value_test_LDADD = libaqbanking.la $(gwenhywfar_libs)
TESTS = testlib ab_value_test
