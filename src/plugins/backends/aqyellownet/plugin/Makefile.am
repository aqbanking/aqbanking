INCLUDES = \
  -I$(top_srcdir) \
  -I$(top_builddir)/headers \
  $(all_includes)

# The directory for the binary-only files (need to define the
# intermediate directory variable to avoid automake complaints
# when installing DATA objects into libdir)
extralibdir = $(libdir)
dist_extralib_DATA=\
 libaqyellownet.so

EXTRA_DIST= aqyellownet.xml.in COPYING

iheaderdir=${includedir}/aqyellownet
iheader_HEADERS=\
 account.h \
 aqyellownet.h \
 provider.h \
 user.h

plugindir=$(aqbanking_plugindir)/providers
plugin_LTLIBRARIES=aqyellownet.la
plugin_DATA=aqyellownet.xml

aqyellownet_la_SOURCES= \
 aqyellownet-plugin.c

aqyellownet_la_LIBADD= \
  -L$(srcdir) -laqyellownet \
  $(aqbanking_internal_libs) \
  $(gwenhywfar_libs)
aqyellownet_la_LDFLAGS=-module

#  -L$(srcdir) -laqyellownet
#  $(srcdir)/libaqyellownet.la

sources:

built_sources: $(BUILT_SOURCES)
