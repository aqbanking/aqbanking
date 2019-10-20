SUBDIRS=msglayer servicelayer transportlayer sessionlayer

AM_CPPFLAGS = -I$(srcdir) -I$(top_srcdir)/src/libs -I$(top_builddir)/src/libs $(gwenhywfar_includes) -I$(srcdir)/..

AM_CFLAGS=-DBUILDING_AQBANKING @visibility_cflags@

EXTRA_DIST=aqfints.xml.in

plugindir=@aqbanking_plugindir@/providers
plugin_DATA=aqfints.xml

noinst_HEADERS=provider_l.h provider_p.h

# static plugin library
noinst_LTLIBRARIES=libaqfints.la 
libaqfints_la_SOURCES=provider.c


libaqfints_la_LIBADD=\
  msglayer/libaqfintsmsg.la \
  servicelayer/libaqfintsservice.la \
  transportlayer/libaqfintstransport.la \
  sessionlayer/libaqfintssession.la



noinst_PROGRAMS = libtest

libtest_SOURCES = libtest.c
libtest_LDADD = libaqfints.la $(top_builddir)/src/libs/aqbanking/libaqbanking_base.la $(gwenhywfar_libs) $(gmp_libs) $(i18n_libs)



sources:
	for f in $(libaqfints_la_SOURCES); do \
	  echo $(subdir)/$$f >>$(top_srcdir)/i18nsources; \
	done
	for f in $(plugin_DATA); do \
	  echo $(subdir)/$$f >>$(top_srcdir)/pdsources; \
	done
	for d in $(SUBDIRS); do \
	  $(MAKE) -C $$d sources; \
	done

built_sources:


cppcheck:
	for f in $(libaqfints_la_SOURCES); do \
	  cppcheck --force $$f ; \
	done
	for d in $(SUBDIRS); do \
	  $(MAKE) -C $$d cppcheck; \
	done


typefiles:
	for d in $(SUBDIRS); do \
	  $(MAKE) -C $$d typefiles; \
	done

typedefs:
	for d in $(SUBDIRS); do \
	  $(MAKE) -C $$d typedefs; \
	done


