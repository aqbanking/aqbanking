#EXTRA_DIST=

AM_CPPFLAGS = -I$(top_srcdir)/src/libs -I$(top_builddir)/src/libs $(gwenhywfar_includes) -I$(srcdir)/..

AM_CFLAGS=@visibility_cflags@

aqcli_libs=${abs_top_builddir}/src/libs/cli/libaqcli.la

bin_PROGRAMS=\
  aqbanking-cli\
  aqhbci-tool4 \
  aqpaypal-tool \
  aqebics-tool \
  aqofxconnect-tool

aqbanking_cli_LDADD= \
  $(aqbanking_internal_libs) \
  $(gwenhywfar_libs) \
  $(aqcli_libs) \
  -lstdc++

aqbanking_cli_SOURCES= \
  main.c \
  addsepadebitnote.c \
  addtransaction.c \
  chkiban.c \
  fillgaps.c \
  import.c \
  export.c \
  listaccs.c \
  listbal.c \
  listconfdir.c \
  listprofiles.c \
  listtrans.c \
  listdoc.c \
  getdoc.c \
  request.c \
  util.c \
  versions.c \
  sepatransfer.c \
  sepainternaltransfer.c \
  sepadebitnote.c \
  sepamultijobs.c \
  separecurtransfer.c \
  updateconf.c \
  control.c \
  accountcmds.c

aqbanking_cli_LDFLAGS=
aqhbci_tool4_LDFLAGS=
aqpaypal_tool_LDFLAGS=
aqebics_tool_LDFLAGS=
aqofxconnect_tool_LDFLAGS=
#-Wl,-static

DEFS += -DLOCALEDIR=\"$(localedir)\" -DAQBANKING_CLI_VERSION_STRING=\"@AQBANKING_CLI_VERSION_STRING@\"

noinst_HEADERS=globals.h
#EXTRA_DIST=



aqhbci_tool4_SOURCES=aqhbci-tool.c
aqhbci_tool4_LDADD= \
  $(aqbanking_internal_libs) \
  $(aqcli_libs) \
  $(gwenhywfar_libs)


aqpaypal_tool_SOURCES=aqpaypal-tool.c
aqpaypal_tool_LDADD= \
  $(aqbanking_internal_libs) \
  $(aqcli_libs) \
  $(gwenhywfar_libs)

aqebics_tool_SOURCES=aqebics-tool.c
aqebics_tool_LDADD= \
  $(aqbanking_internal_libs) \
  $(aqcli_libs) \
  $(gwenhywfar_libs)

aqofxconnect_tool_SOURCES=aqofxconnect-tool.c
aqofxconnect_tool_LDADD= \
  $(aqbanking_internal_libs) \
  $(aqcli_libs) \
  $(gwenhywfar_libs)


if IS_WINDOWS

aqbanking_cli_LDFLAGS+=-mconsole
aqhbci_tool4_LDFLAGS+=-mconsole
aqpaypal_tool_LDFLAGS+=-mconsole
aqebics_tool_LDFLAGS+=-mconsole
aqofxconnect_tool_LDFLAGS+=-mconsole

endif

sources:
	for f in $(aqbanking_cli_SOURCES) $(aqhbci_tool4_SOURCES) $(aqpaypal_tool_SOURCES) $(aqebics_tool_SOURCES); $(aqofxconnect_tool_SOURCES); do \
	  echo $(subdir)/$$f >>$(top_srcdir)/i18nsources; \
	done


built_sources:
	for d in $(SUBDIRS); do \
	  $(MAKE) -C $$d built_sources; \
	done ;


typefiles:

typedefs:


cppcheck:
	for f in $(aqbanking_cli_SOURCES); do \
	  cppcheck --force $$f ; \
	done
	for d in $(SUBDIRS); do \
	  $(MAKE) -C $$d cppcheck; \
	done


