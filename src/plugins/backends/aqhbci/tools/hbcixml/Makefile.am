INCLUDES = -I$(aqbanking_symlinkdir) $(all_includes)

DEFS += -DXMLDATA_DIR=\"$(aqbanking_pkgdatadir)/backends/aqhbci\"

bin_PROGRAMS=hbcixml3
noinst_HEADERS=loganalyzer.h error.h pointer.h
hbcixml3_SOURCES=hbcixml.cpp loganalyzer.cpp error.cpp
hbcixml3_LDADD = \
  $(aqhbci_internal_libs) \
  $(aqbanking_internal_libs) \
  $(gwenhywfar_libs) -lstdc++

if IS_WINDOWS

hbcixml3_LDFLAGS = -mconsole

endif

