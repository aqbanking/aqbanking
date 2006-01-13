SUBDIRS=msglayer joblayer accountjobs applayer banking

AM_CFLAGS=-DBUILDING_AQHBCI @visibility_cflags@

EXTRA_DIST=aqhbci.xml.in header.xml.in

INCLUDES = -I$(top_builddir)/headers $(all_includes) \
 -I$(srcdir)/joblayer

BUILT_SOURCES = version.h

CLEANFILES = $(BUILT_SOURCES) hbci.xml

plugindir= $(aqbanking_plugindir)/providers
plugin_LTLIBRARIES=aqhbci.la
if IS_WINDOWS
plugin_DATA = aqhbci.xml 
else
plugin_DATA=aqhbci.xml
endif

iheaderdir=${includedir}/aqhbci
iheader_HEADERS=aqhbci.h

noinst_HEADERS = aqhbci_l.h

lib_LTLIBRARIES=libaqhbci.la
libaqhbci_la_SOURCES=dummy.c

xmldatadir = $(aqhbci_data)/xml
xmldata_DATA=hbci.xml

hbci.xml: header.xml applayer/xml/base.xml accountjobs/accountjobs.xml
	$(XMLMERGE) --compact --header -v header.xml applayer/xml/base.xml accountjobs/accountjobs.xml -o $@


sources:
	for f in $(libaqhbci_la_SOURCES); do \
	  echo $(subdir)/$$f >>$(top_srcdir)/i18nsources; \
	done
	for d in $(SUBDIRS); do \
	  make -C $$d sources; \
	done

libaqhbci_la_LIBADD = \
  msglayer/libhbcimsg.la \
  joblayer/libhbcijob.la \
  accountjobs/libhbciaccjobs.la \
  applayer/libhbciapp.la \
  banking/libhbcibanking.la \
  $(aqbanking_internal_libs) \
  $(gwenhywfar_libs)

libaqhbci_la_LDFLAGS = -no-undefined \
  -version-info @AQHBCI_SO_CURRENT@:@AQHBCI_SO_REVISION@:@AQHBCI_SO_AGE@


aqhbci_la_SOURCES=aqhbci-plugin.c
aqhbci_la_LIBADD = \
  libaqhbci.la \
  $(aqbanking_internal_libs) \
  $(gwenhywfar_libs)

aqhbci_la_LDFLAGS = -no-undefined -module \
  -version-info @AQHBCI_SO_CURRENT@:@AQHBCI_SO_REVISION@:@AQHBCI_SO_AGE@


# Add the linker flags for windows resource file
libaqhbci_la_DEPENDENCIES = msglayer/libhbcimsg.la \
  joblayer/libhbcijob.la accountjobs/libhbciaccjobs.la \
  applayer/libhbciapp.la banking/libhbcibanking.la \
  $(aqbanking_internal_libs)
aqhbci_la_DEPENDENCIES = libaqhbci.la $(aqbanking_internal_libs)
# if IS_WINDOWS
if IS_WINDOWS
libaqhbci_la_DEPENDENCIES += ressource.o
aqhbci_la_DEPENDENCIES += ressource.o
libaqhbci_la_LDFLAGS += -Wl,ressource.o
aqhbci_la_LDFLAGS += -Wl,ressource.o
ressource.o: ressource.rc
	$(RC) -o $@ $<
endif
# IS_WINDOWS
# End of linker flags for windows resource file

built_sources: $(BUILT_SOURCES)

