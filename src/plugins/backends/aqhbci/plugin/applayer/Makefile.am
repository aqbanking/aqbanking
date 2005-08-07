SUBDIRS=xml accountjobs

INCLUDES = -I$(top_builddir)/headers $(all_includes) \
  -I$(srcdir)/accountjobs \
  -I$(srcdir)/../joblayer -I$(srcdir)/../msglayer -I$(srcdir)/..

EXTRA_DIST=README header.xml.in

noinst_LTLIBRARIES=libhbciapp.la

noinst_HEADERS=\
  adminjobs_p.h\
  outbox_l.h \
  outbox_p.h

iheaderdir=${includedir}/aqhbci
iheader_HEADERS=\
  adminjobs.h \
  outbox.h

libhbciapp_la_SOURCES=\
  adminjobs.c \
  outbox.c

sources:
	for f in $(libhbciapp_la_SOURCES); do \
	  echo $(subdir)/$$f >>$(top_srcdir)/i18nsources; \
	done
	for d in $(SUBDIRS); do \
	  make -C $$d sources; \
	done

libhbciapp_la_LIBADD= accountjobs/libhbciaccjobs.la

xmldatadir = $(aqhbci_data)/xml
xmldata_DATA=hbci.xml

hbci.xml: header.xml xml/base.xml accountjobs/accountjobs.xml
	$(XMLMERGE) --compact --header -v header.xml xml/base.xml  accountjobs/accountjobs.xml -o $@

CLEANFILES = $(xmldata_DATA) tmp.xml
