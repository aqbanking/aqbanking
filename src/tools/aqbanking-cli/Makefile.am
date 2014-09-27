#EXTRA_DIST=

INCLUDES = -I$(aqbanking_symlinkdir) $(gwenhywfar_includes) $(aqfinance_includes) \
  -I$(top_srcdir)/src/plugins/backends/aqhbci/plugin/banking \
  -I$(top_srcdir)/src/plugins/backends/aqhbci/plugin/msglayer

AM_CFLAGS=@visibility_cflags@

bin_PROGRAMS=aqbanking-cli

aqbanking_cli_LDADD= \
  $(aqhbci_internal_libs) \
  $(aqbanking_internal_libs) \
  $(gwenhywfar_libs) -lstdc++

aqbanking_cli_SOURCES= \
  main.c
aqbanking_cli_LDFLAGS=
#-Wl,-static

DEFS += -DLOCALEDIR=\"$(localedir)\" -DAQBANKING_CLI_VERSION_STRING=\"@AQBANKING_CLI_VERSION_STRING@\"

noinst_HEADERS=globals.h
EXTRA_DIST=\
  addsepadebitnote.c \
  addtransaction.c \
  chkacc.c \
  chkiban.c \
  debitnote.c \
  debitnotes.c \
  fillgaps.c \
  import.c \
  listaccs.c \
  listbal.c \
  listprofiles.c \
  listtrans.c \
  listtransfers.c \
  request.c \
  senddtazv.c \
  transfer.c \
  transfers.c \
  util.c \
  versions.c \
  updateconf.c \
  sepatransfer.c \
  sepadebitnote.c \
  sepamultijobs.c \
  separecurtransfer.c


if IS_WINDOWS

aqbanking_cli_LDFLAGS+=-mconsole

endif

sources:
	for f in $(aqbanking_cli_SOURCES); do \
	  echo $(subdir)/$$f >>$(top_srcdir)/i18nsources; \
	done


built_sources:
	for d in $(SUBDIRS); do \
	  $(MAKE) -C $$d built_sources; \
	done ;

