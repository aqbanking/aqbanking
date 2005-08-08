INCLUDES = -I$(top_builddir)/headers $(all_includes)

DEFS += -DXMLDATA_DIR=\"$(aqhbci_data)/xml\"

bin_PROGRAMS=hbcixml2
noinst_HEADERS=loganalyzer.h error.h pointer.h
hbcixml2_SOURCES=hbcixml.cpp loganalyzer.cpp error.cpp
hbcixml2_LDADD = \
  -L$(top_builddir)/src/plugins/backends/aqhbci/plugin $(aqhbci_internal_libs) \
  -L$(top_builddir)/src/libs/aqbanking $(aqbanking_libs) \
  $(gwenhywfar_libs)
