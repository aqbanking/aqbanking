INCLUDES = -I$(top_builddir)/headers $(all_includes)
bin_PROGRAMS=aqhbci-tool

localedir = $(datadir)/locale
DEFS += -DLOCALEDIR=\"$(localedir)\"

noinst_HEADERS=\
  cbanking_p.h cbanking_l.h cbanking.h \
  progress_p.h progress_l.h globals.h

aqhbci_tool_SOURCES=main.c cbanking.c progress.c \
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

aqhbci_tool_LDADD = -L$(top_builddir)/src/libs $(aqhbci_internal_libs) \
  $(gwenhywfar_libs) -L$(top_builddir)/src/libs/aqbanking $(aqbanking_libs)

sources:

