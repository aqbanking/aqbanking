INCLUDES=@all_includes@


noinst_PROGRAMS=abtest cpptest

abtest_SOURCES=abtest.c
abtest_LDADD = -L$(top_builddir)/src/libs/aqbanking $(aqbanking_internal_libs) $(gwenhywfar_libs)

cpptest_SOURCES=cpptest.cpp
cpptest_LDADD = -L$(top_builddir)/src/libs/aqbanking $(aqbanking_internal_libs) $(top_builddir)/src/libs/aqbanking++/libaqbankingpp.la $(gwenhywfar_libs)

