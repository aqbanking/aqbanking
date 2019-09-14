DIST_SUBDIRS=msglayer joblayer ajobs admjobs applayer banking tan dialogs control
SUBDIRS=msglayer joblayer ajobs admjobs applayer banking tan dialogs control

AM_CFLAGS=-DBUILDING_AQHBCI @visibility_cflags@

EXTRA_DIST=aqhbci.xml.in header.xml.in

INCLUDES = -I$(top_srcdir)/src/libs -I$(top_builddir)/src/libs $(gwenhywfar_includes) \
 -I$(srcdir)/../ \
 -I$(srcdir)/joblayer

BUILT_SOURCES = version.h

CLEANFILES = $(BUILT_SOURCES) hbci.xml

plugindir= $(aqbanking_plugindir)/providers
plugin_DATA=aqhbci.xml

#iheaderdir=@aqbanking_headerdir_am@/aqhbci
#iheader_HEADERS=

noinst_HEADERS = \
  aqhbci.h \
  aqhbci_l.h

xmldatadir=$(aqbanking_pkgdatadir)/backends/aqhbci
xmldata_DATA=hbci.xml

hbci.xml: header.xml applayer/xml/base.xml ajobs/accountjobs.xml admjobs/adminjobs.xml
	$(XMLMERGE) --compact --header -v header.xml applayer/xml/base.xml ajobs/accountjobs.xml admjobs/adminjobs.xml -o $@


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

# AqHBCI plugin library
noinst_LTLIBRARIES=libaqhbci.la
libaqhbci_la_SOURCES=aqhbci.c
libaqhbci_la_LIBADD=\
  msglayer/libhbcimsg.la \
  joblayer/libhbcijob.la \
  ajobs/libhbciaccjobs.la \
  admjobs/libhbciadmjobs.la \
  applayer/libhbciapp.la \
  banking/libhbcibanking.la \
  control/libhbcicontrol.la \
  dialogs/libaqhbci_dlgs.la \
  tan/libhbcitan.la


built_sources: $(BUILT_SOURCES)


cppcheck:
	for f in $(libaqhbci_la_SOURCES); do \
	  cppcheck --force $$f ; \
	done
	for d in $(SUBDIRS); do \
	  $(MAKE) -C $$d cppcheck; \
	done


types:

typedefs:

