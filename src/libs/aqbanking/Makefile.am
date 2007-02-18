SUBDIRS=types jobs

INCLUDES = -I$(top_builddir)/headers -I$(srcdir)/types -I$(srcdir)/jobs $(all_includes)

lib_LTLIBRARIES=libaqbanking.la

AM_CFLAGS=-DBUILDING_AQBANKING @visibility_cflags@


# We define those here because within the configure script the variables 
# $(datadir) et al are not available.
localedir=$(datadir)/locale
DEFS += \
 -DEXEEXT=\"$(EXEEXT)\" \
 -DLOCALEDIR=\"$(localedir)\" \
 -DAQBANKING_PLUGINS=\"$(aqbanking_plugindir)\" \
 -DAQBANKING_SYSCONF_DIR=\"$(sysconfdir)\" \
 -DAQBANKING_DATA_DIR=\"$(pkgdatadir)\" \
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
 cryptmanager_l.h \
 cryptmanager_p.h \
 httpsession_p.h \
 i18n_l.h \
 imexporter_l.h \
 imexporter_p.h \
 job_l.h \
 job_p.h \
 provider_l.h \
 provider_p.h \
 wcb_l.h \
 wcb_p.h \
 msgengine_p.h


iheaderdir=${includedir}/aqbanking
iheader_HEADERS=\
 account.h \
 account_be.h \
 banking.h \
 banking_be.h \
 banking_imex.h \
 banking_info.h \
 banking_ob.h \
 banking_simple.h \
 banking_virt.h \
 bankinfoplugin.h \
 bankinfoplugin_be.h \
 country.h \
 error.h \
 httpsession.h \
 imexporter.h \
 imexporter_be.h \
 job.h \
 job_be.h \
 provider.h \
 provider_be.h\
 msgengine.h


libaqbanking_la_SOURCES=\
 account.c \
 bankinfoplugin.c \
 banking.c \
 country.c \
 cryptmanager.c \
 httpsession.c \
 imexporter.c \
 job.c \
 provider.c \
 wcb.c\
 msgengine.c


libaqbanking_la_LIBADD= $(gwenhywfar_libs) \
 jobs/libjobs.la \
 types/libtypes.la

libaqbanking_la_LDFLAGS = -no-undefined \
  -version-info @AQBANKING_SO_CURRENT@:@AQBANKING_SO_REVISION@:@AQBANKING_SO_AGE@

# Add the linker flags for windows resource file
libaqbanking_la_DEPENDENCIES = \
 jobs/libjobs.la \
 types/libtypes.la
# if IS_WINDOWS
if IS_WINDOWS
libaqbanking_la_DEPENDENCIES += ressource.o
libaqbanking_la_LDFLAGS += -Wl,ressource.o
ressource.o: ressource.rc
	$(RC) -o $@ $<
endif
# IS_WINDOWS
# End of linker flags for windows resource file

sources:
	for f in $(libaqbanking_la_SOURCES); do \
	  echo $(subdir)/$$f >>$(top_srcdir)/i18nsources; \
	done
	for d in $(SUBDIRS); do \
	  $(MAKE) -C $$d sources; \
	done


# Build and link a test program to verify the linker flags
noinst_PROGRAMS = testlib
testlib_SOURCES = testlib.c
testlib_LDADD = libaqbanking.la
TESTS = testlib
