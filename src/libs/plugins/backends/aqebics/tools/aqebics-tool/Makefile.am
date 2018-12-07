EXTRA_DIST=README

INCLUDES=-I.. -I$(top_builddir) -Iclient -I$(builddir)/client \
  $(gwenhywfar_includes) -I$(aqbanking_symlinkdir) $(AQEBICS_CFLAGS)

bin_PROGRAMS=aqebics-tool

idatadir=$(datadir)/doc/aqebics/aqebics-tool
idata_DATA=README

localedir = $(datadir)/locale
DEFS += -DLOCALEDIR=\"$(localedir)\"

noinst_HEADERS=globals.h

aqebics_tool_SOURCES=main.c \
 addaccount.c \
 addsubuserflags.c \
 adduser.c \
 createkeys.c \
 createtempkey.c \
 download.c \
 sendkeys.c \
 sendsignkey.c \
 getaccounts.c \
 getkeys.c \
 iniletter.c \
 hialetter.c \
 sendhpd.c \
 sendhkd.c \
 upload.c \
 utils.c \
 mkpinlist.c \
 resetuser.c \
 getcert.c \
 listusers.c \
 listaccounts.c \
 setebicsversion.c


aqebics_tool_LDADD = \
  $(aqebics_internal_libs) \
  $(aqbanking_internal_libs) \
  $(gwenhywfar_libs) \
  $(aqebics_static_plugins) \
  $(aqebics_extra_libs)


sources:
	for d in $(SUBDIRS); do \
	  $(MAKE) -C $$d sources; \
	done
	for f in $(aqebics_tool_SOURCES); do \
	  echo $(subdir)/$$f >>$(top_srcdir)/i18nsources; \
	done

