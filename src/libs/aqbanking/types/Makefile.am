INCLUDES=-I$(aqbanking_symlinkdir) @all_includes@

AM_CFLAGS=-DBUILDING_AQBANKING @visibility_cflags@

noinst_LTLIBRARIES=libtypes.la

typefiles=\
  user.xml \
  cellphone.xml


noinst_HEADERS=\
  user_l.h \
  user_p.h \
  userfns_l.h \
  value_p.h \
  cellphoneproduct_l.h \
  cellphoneproduct_p.h


iheaderdir=@aqbanking_headerdir_am@/aqbanking
iheader_HEADERS=\
  user.h \
  userfns.h \
  value.h \
  cellphoneproduct.h

libtypes_la_SOURCES=\
  user.c \
  userfns.c \
  value.c \
  cellphoneproduct.c


types: $(typefiles)
	$(gwenhywfar_bindir)/typemaker -d AQBANKING_API $(typefiles)


sources:
	for f in $(libtypes_la_SOURCES); do \
	  echo $(subdir)/$$f >>$(top_srcdir)/i18nsources; \
	done

