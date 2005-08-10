INCLUDES = -I$(top_builddir)/headers $(all_includes)
bin_PROGRAMS=aqhbci-tool

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
 getaccounts.c

aqhbci_tool_LDADD = \
  -L$(top_builddir)/src/plugins/backends/aqhbci/plugin $(aqhbci_internal_libs) \
  -L$(top_builddir)/src/libs/aqbanking $(aqbanking_libs) \
  -L$(top_builddir)/src/frontends/cbanking -lcbanking \
  $(gwenhywfar_libs)

sources:

