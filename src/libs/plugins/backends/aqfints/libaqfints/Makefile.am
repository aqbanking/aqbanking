SUBDIRS=parser msglayer servicelayer transportlayer sessionlayer

AM_CPPFLAGS = -I$(srcdir) -I$(top_srcdir)/src/lib -I$(top_builddir)/src/lib $(gwenhywfar_includes) $(aqbanking_includes) -I$(srcdir)/..

AM_CFLAGS=@visibility_cflags@

# static plugin library
noinst_LTLIBRARIES=libaqfints.la 
libaqfints_la_SOURCES=\
  aqfints.c


noinst_HEADERS= \
  aqfints.h


libaqfints_la_LIBADD= $(gwenhywfar_libs) $(aqbanking_libs) \
  parser/libaqfintsparser.la \
  msglayer/libaqfintsmsg.la \
  servicelayer/libaqfintsservice.la \
  transportlayer/libaqfintstransport.la \
  sessionlayer/libaqfintssession.la




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


