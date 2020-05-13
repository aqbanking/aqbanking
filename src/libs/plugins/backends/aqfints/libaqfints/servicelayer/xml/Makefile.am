
AM_CFLAGS=@visibility_cflags@

AM_CPPFLAGS = -I$(top_srcdir)/src/libs -I$(top_builddir)/src/libs $(gwenhywfar_includes) \
  -I$(srcdir)/../.. \
  -I$(srcdir)/.. \
  $(all_includes)


fintsdatadir=$(aqfints_datadir)/fints
fintsdata_DATA=\
  basic.fints \
  crypt.fints \
  ident.fints \
  prepare.fints \
  dialogend.fints \
  tan.fints \
  sync.fints



typefiles:
	for d in $(SUBDIRS); do \
	  $(MAKE) -C $$d typefiles; \
	done

typedefs:
	for d in $(SUBDIRS); do \
	  $(MAKE) -C $$d typedefs; \
	done
