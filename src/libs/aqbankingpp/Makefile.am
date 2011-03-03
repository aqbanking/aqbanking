INCLUDES=-I$(aqbanking_symlinkdir) $(gwenhywfar_includes)

lib_LTLIBRARIES=libaqbankingpp.la

noinst_HEADERS=

if IS_WINDOWS
  libaqbankingpp_la_CPPFLAGS = -Dexport_AQBANKINGPP $(AM_CPPFLAGS)
endif

iheaderdir=@aqbanking_headerdir_am@/aqbankingpp
iheader_HEADERS=\
 accountstatus.hpp \
 aqbankingppdecl.hpp \
 balance.hpp \
 cxxwrap.hpp \
 stringlist.hpp \
 time.hpp \
 value.hpp

libaqbankingpp_la_SOURCES=\
 stringlist.cpp

libaqbankingpp_la_LIBADD = \
 $(aqbanking_internal_libs) \
 $(gwenhywfar_libs) $(i18n_libs)
libaqbankingpp_la_LDFLAGS =


sources:
	for f in $(libaqbankingpp_la_SOURCES); do \
	  echo $(subdir)/$$f >>$(top_srcdir)/i18nsources; \
	done


# Build and link a test program to verify the linker flags
noinst_PROGRAMS = testlibcc

# Test program to verify the c++ usage
testlibcc_SOURCES = testlibcc.cpp
testlibcc_LDADD = $(aqbanking_internal_libs) $(gwenhywfar_libs)

TESTS = testlibcc
