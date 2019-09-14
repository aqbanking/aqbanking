
AM_CFLAGS=-DBUILDING_AQHBCI @visibility_cflags@

INCLUDES = -I$(top_srcdir)/src/libs -I$(top_builddir)/src/libs $(gwenhywfar_includes) \
  -I$(srcdir)/../.. \
  -I$(srcdir)/../msglayer -I$(srcdir)/..

typefiles=tanmethod.xml

noinst_LTLIBRARIES=libhbcitan.la

noinst_HEADERS=\
  tanmethod_l.h \
  tanmethod_p.h \
  tanmethod.h \
  tanmechanism.h tanmechanism_p.h \
  tan_chiptan_opt.h \
  tan_text.h \
  tan_image.h


#iheaderdir=@aqbanking_headerdir_am@/aqhbci
#iheader_HEADERS=

libhbcitan_la_SOURCES=\
  tanmethod.c \
  tanmechanism.c \
  tan_chiptan_opt.c \
  tan_image.c \
  tan_text.c


EXTRA_DIST=$(typefiles)




types: $(typefiles)
	$(gwenhywfar_bindir)/typemaker -d AQHBCI_API $(typefiles)

sources:
	for f in $(libhbcitan_la_SOURCES); do \
	  echo $(subdir)/$$f >>$(top_srcdir)/i18nsources; \
	done
	for d in $(SUBDIRS); do \
	  $(MAKE) -C $$d sources; \
	done


cppcheck:
	for f in $(libhbcitan_la_SOURCES); do \
	  cppcheck --force $$f ; \
	done
	for d in $(SUBDIRS); do \
	  $(MAKE) -C $$d cppcheck; \
	done


