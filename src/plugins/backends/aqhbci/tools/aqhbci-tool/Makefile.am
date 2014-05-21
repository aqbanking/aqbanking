EXTRA_DIST=README

INCLUDES = -I$(aqbanking_symlinkdir) $(all_includes)
bin_PROGRAMS=aqhbci-tool4

idatadir=$(datadir)/doc/aqhbci/aqhbci-tool
idata_DATA=README

localedir = $(datadir)/locale
DEFS += -DLOCALEDIR=\"$(localedir)\" -DXMLDATA_DIR=\"$(aqbanking_pkgdatadir)/backends/aqhbci\"


noinst_HEADERS=globals.h

aqhbci_tool4_SOURCES=main.c \
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
 delaccount.c \
 getcert.c \
 sethbciversion.c \
 adduserflags.c \
 subuserflags.c \
 listitanmodes.c \
 setitanmode.c \
 setmaxtransfers.c \
 setsepaprofile.c \
 settanmediumid.c \
 logfile.c \
 getaccsepa.c


aqhbci_tool4_LDADD = \
  $(aqhbci_internal_libs) \
  $(aqbanking_internal_libs) \
  $(gwenhywfar_libs) -lstdc++


if IS_WINDOWS

aqhbci_tool4_LDFLAGS=-mconsole

endif

sources:

