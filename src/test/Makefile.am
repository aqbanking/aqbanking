INCLUDES=@all_includes@


noinst_PROGRAMS=abtest cpptest

abtest_SOURCES=abtest.c
abtest_LDADD = -L$(top_builddir)/src/libs/aqbanking -laqbanking $(gwenhywfar_libs)

cpptest_SOURCES=cpptest.cpp
cpptest_LDADD = -L$(top_builddir)/src/libs/aqbanking -laqbanking -L$(top_builddir)/src/libs/aqbanking++ -laqbankingpp $(gwenhywfar_libs)

