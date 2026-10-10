AM_CPPFLAGS = $(gwenhywfar_includes) -I.. -I$(top_srcdir)/src/libs -I$(top_builddir)/src/libs

noinst_LTLIBRARIES=libaqcli.la

# Build and link a library for cli usage
libaqcli_la_SOURCES = helper.c
libaqcli_la_LIBADD = $(gwenhywfar_libs)

EXTRA_DIST=\
 helper.h

noinst_HEADERS=\
 helper.h

cppcheck:
	for f in $(libaqcli_la_SOURCES); do \
	  cppcheck --force $$f ; \
	done


