DIST_SUBDIRS=aqbanking aqbankingpp plugins
SUBDIRS=aqbanking plugins

INCLUDES = $(gwenhywfar_includes) -I$(aqbanking_symlinkdir)


lib_LTLIBRARIES=libaqbanking.la

libaqbanking_la_SOURCES= dummy.c
libaqbanking_la_LDFLAGS = -no-undefined -version-info @AQBANKING_SO_CURRENT@:@AQBANKING_SO_REVISION@:@AQBANKING_SO_AGE@
libaqbanking_la_LIBADD= $(gwenhywfar_libs) $(gmp_libs) $(i18n_libs) \
  aqbanking/libaqbanking_base.la \
  plugins/libabplugins.la



noinst_PROGRAMS = testlib ab_value_test

# Build and link a test program to verify the linker flags
testlib_SOURCES = testlib.c
testlib_LDADD = libaqbanking.la $(gwenhywfar_libs)

# Another test program to verify the parsing of decimal numbers
ab_value_test_SOURCES = ab-value-test.c
ab_value_test_LDADD = libaqbanking.la $(gwenhywfar_libs)


TESTS = testlib ab_value_test



sources:
	for d in $(SUBDIRS); do \
	  $(MAKE) -C $$d sources; \
	done

built_sources:
	for d in $(SUBDIRS); do \
	  $(MAKE) -C $$d built_sources; \
	done ;
