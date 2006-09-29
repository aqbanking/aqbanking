EXTRA_DIST=README

INCLUDES = -I$(top_builddir)/headers $(all_includes)
bin_PROGRAMS=aqhbci-tool

idatadir=$(datadir)/doc/aqhbci/aqhbci-tool
idata_DATA=README

localedir = $(datadir)/locale
DEFS += -DLOCALEDIR=\"$(localedir)\"

noinst_HEADERS=globals.h

aqhbci_tool_SOURCES=main.c \
 mkpinlist.c \
 addmedium.c \
 listmedia.c \
 adduser.c \
 getsysid.c \
 getkeys.c \
 resetkeys.c \
 createkeys.c \
 sendkeys.c \
 getaccounts.c \
 listusers.c \
 listaccounts.c \
 iniletter.c \
 getitanmodes.c \
 changepin.c \
 test.c

aqhbci_tool_LDADD = \
  $(aqhbci_internal_libs) \
  $(cbanking_internal_libs) \
  $(aqbanking_internal_libs) \
  $(gwenhywfar_libs)

sources:

