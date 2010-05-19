EXTRA_DIST=README

INCLUDES=-I$(aqbanking_symlinkdir) $(gwenhywfar_includes)
bin_PROGRAMS=aqpaypal-tool

idatadir=$(datadir)/doc/aqpaypal/aqpaypal-tool
idata_DATA=README

localedir = $(datadir)/locale
DEFS += -DLOCALEDIR=\"$(localedir)\"

noinst_HEADERS=globals.h

aqpaypal_tool_SOURCES=main.c \
 listusers.c \
 listaccounts.c \
 adduser.c \
 addaccount.c \
 setsecrets.c


aqpaypal_tool_LDADD = \
  $(aqpaypal_internal_libs) \
  $(aqbanking_internal_libs) \
  $(gwenhywfar_libs)


sources:
	for d in $(SUBDIRS); do \
	  $(MAKE) -C $$d sources; \
	done
	for f in $(aqpaypal_tool_SOURCES); do \
	  echo $(subdir)/$$f >>$(top_srcdir)/i18nsources; \
	done


built_sources:

