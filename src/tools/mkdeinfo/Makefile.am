INCLUDES = -I$(aqbanking_symlinkdir) $(gwenhywfar_includes) \
           -I$(top_srcdir)/src/libs/aqbanking




if IS_WINDOWS
# Doesn't link on windows; too lazy to fix this for now.
noinst_PROGRAMS=
else
noinst_PROGRAMS=mkdeinfo
mkdeinfo_SOURCES=mkdeinfo.c
mkdeinfo_LDADD = $(aqbanking_internal_libs) $(gwenhywfar_libs)
endif
# IS_WINDOWS



sources:


cppcheck:
	for f in $(mkdeinfo_SOURCES); do \
	  cppcheck --force $$f ; \
	done
	for d in $(SUBDIRS); do \
	  $(MAKE) -C $$d cppcheck; \
	done

