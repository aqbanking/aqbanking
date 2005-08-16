INCLUDES = -I$(top_builddir)/headers $(all_includes)

DEFS += -DXMLDATA_DIR=\"$(aqhbci_data)/xml\"

bin_PROGRAMS=hbcixml2
noinst_HEADERS=loganalyzer.h error.h pointer.h
hbcixml2_SOURCES=hbcixml.cpp loganalyzer.cpp error.cpp
hbcixml2_LDADD = \
  $(aqhbci_internal_libs) \
  $(aqbanking_internal_libs) \
  $(gwenhywfar_libs)
