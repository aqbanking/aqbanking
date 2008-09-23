EXTRA_DIST=README

INCLUDES = -I$(top_builddir)/headers $(all_includes)
bin_PROGRAMS=aqhbci-tool3

idatadir=$(datadir)/doc/aqhbci/aqhbci-tool
idata_DATA=README

localedir = $(datadir)/locale
DEFS += -DLOCALEDIR=\"$(localedir)\"

noinst_HEADERS=globals.h

aqhbci_tool3_SOURCES=main.c \
 mkpinlist.c \
 adduser.c \
 deluser.c \
 addsubaccountflags.c \
 getsysid.c \
 getkeys.c \
 createkeys.c \
 sendkeys.c \
 getaccounts.c \
 listusers.c \
 listaccounts.c \
 iniletter.c \
 getitanmodes.c \
 changepin.c \
 test.c \
 addaccount.c \
 getcert.c \
 sethbciversion.c \
 adduserflags.c \
 subuserflags.c

aqhbci_tool3_LDADD = \
  $(aqhbci_internal_libs) \
  $(cbanking_internal_libs) \
  $(aqbanking_internal_libs) \
  $(gwenhywfar_libs) $(aqb_static_plugins) $(aqb_extra_libs)


if IS_WINDOWS

aqhbci_tool3_LDFLAGS=-mconsole

endif

sources:

