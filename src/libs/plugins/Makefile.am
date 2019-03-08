SUBDIRS=parsers imexporters bankinfo backends


noinst_LTLIBRARIES=libabplugins.la
libabplugins_la_SOURCES=dummy.c
libabplugins_la_LIBADD=\
  backends/libabbackends.la \
  bankinfo/libabbankinfo.la \
  imexporters/libabimexporters.la



sources:
	for d in $(SUBDIRS); do \
	  $(MAKE) -C $$d sources; \
	done


built_sources:
	for d in $(SUBDIRS); do \
	  $(MAKE) -C $$d built_sources; \
	done ;


cppcheck:
	for d in $(SUBDIRS); do \
	  $(MAKE) -C $$d cppcheck; \
	done


types:
	for d in $(SUBDIRS); do \
	  $(MAKE) -C $$d types; \
	done

typedefs:
	for d in $(SUBDIRS); do \
	  $(MAKE) -C $$d typedefs; \
	done

