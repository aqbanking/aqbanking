SUBDIRS=msglayer joblayer ajobs applayer banking dialogs

AM_CFLAGS=-DBUILDING_AQHBCI @visibility_cflags@

EXTRA_DIST=aqhbci.xml.in header.xml.in

INCLUDES = -I$(aqbanking_symlinkdir) $(gwenhywfar_includes) \
 -I$(srcdir)/joblayer

BUILT_SOURCES = version.h

CLEANFILES = $(BUILT_SOURCES) hbci.xml

plugindir= $(aqbanking_plugindir)/providers
plugin_LTLIBRARIES=aqhbci.la
plugin_DATA=aqhbci.xml

iheaderdir=@aqbanking_headerdir_am@/aqhbci
iheader_HEADERS=aqhbci.h

noinst_HEADERS = aqhbci_l.h

noinst_PROGRAMS=testlib

xmldatadir=$(aqbanking_pkgdatadir)/backends/aqhbci
xmldata_DATA=hbci.xml

hbci.xml: header.xml applayer/xml/base.xml ajobs/accountjobs.xml
	$(XMLMERGE) --compact --header -v header.xml applayer/xml/base.xml ajobs/accountjobs.xml -o $@


sources:
	for f in $(libaqhbci_la_SOURCES); do \
	  echo $(subdir)/$$f >>$(top_srcdir)/i18nsources; \
	done
	for f in $(plugin_DATA); do \
	  echo $(subdir)/$$f >>$(top_srcdir)/pdsources; \
	done
	for d in $(SUBDIRS); do \
	  $(MAKE) -C $$d sources; \
	done

# libaqhbci
lib_LTLIBRARIES=libaqhbci.la
libaqhbci_la_SOURCES=aqhbci.c
libaqhbci_la_LIBADD = \
  msglayer/libhbcimsg.la \
  joblayer/libhbcijob.la \
  ajobs/libhbciaccjobs.la \
  applayer/libhbciapp.la \
  banking/libhbcibanking.la \
  dialogs/libaqhbci_dlgs.la \
  $(aqbanking_internal_libs) \
  $(gwenhywfar_libs)
libaqhbci_la_LDFLAGS = -no-undefined \
  -version-info @AQHBCI_SO_CURRENT@:@AQHBCI_SO_REVISION@:@AQHBCI_SO_AGE@

# AqHBCI plugin library
noinst_LTLIBRARIES=libaqhbciplugin.la
libaqhbciplugin_la_SOURCES=aqhbci-plugin.c
libaqhbciplugin_la_LIBADD=\
  libaqhbci.la \
  $(aqbanking_internal_libs) \
  $(gwenhywfar_libs) 

# AqHBCI module
aqhbci_la_SOURCES=dummy.c
aqhbci_la_LIBADD = \
  libaqhbciplugin.la \
  $(aqbanking_internal_libs) \
  $(gwenhywfar_libs) 
aqhbci_la_LDFLAGS = -no-undefined -module -avoid-version


testlib_SOURCES=testlib.c
testlib_LDADD = \
  $(aqhbci_internal_libs) \
  $(aqbanking_internal_libs) \
  $(gwenhywfar_libs)


built_sources: $(BUILT_SOURCES)

