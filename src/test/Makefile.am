INCLUDES = -I$(top_builddir)/headers $(all_includes)


noinst_PROGRAMS=abtest 

abtest_SOURCES=abtest.c
abtest_LDADD = $(aqbanking_internal_libs) $(gwenhywfar_libs)

#cpptest_SOURCES=cpptest.cpp
#cpptest_LDADD = $(aqbanking_internal_libs) $(top_builddir)/src/libs/aqbanking++/libaqbankingpp.la $(gwenhywfar_libs) -lstdc++

sources:
	for f in $(abtest_la_SOURCES); do \
	  echo $(subdir)/$$f >>$(top_srcdir)/i18nsources; \
	done

built_sources:

