INCLUDES=-I$(aqbanking_symlinkdir) @all_includes@

AM_CFLAGS=-DBUILDING_AQBANKING @visibility_cflags@

noinst_LTLIBRARIES=libtypes.la

typefiles=\
  cellphone.xml


noinst_HEADERS=\
  value_p.h \
  cellphoneproduct.h \
  cellphoneproduct_l.h \
  cellphoneproduct_p.h


iheaderdir=@aqbanking_headerdir_am@/aqbanking
iheader_HEADERS=\
  value.h

libtypes_la_SOURCES=\
  value.c \
  cellphoneproduct.c


types: $(typefiles)
	$(gwenhywfar_bindir)/typemaker -d AQBANKING_API $(typefiles)

typedefs:


sources:
	for f in $(libtypes_la_SOURCES); do \
	  echo $(subdir)/$$f >>$(top_srcdir)/i18nsources; \
	done



cppcheck:
	for f in $(libabtypes_la_SOURCES); do \
	  cppcheck --force $$f ; \
	done

