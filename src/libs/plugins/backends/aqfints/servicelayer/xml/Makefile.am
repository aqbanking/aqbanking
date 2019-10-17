
fintsdatadir=$(aqbanking_pkgdatadir)/backends/aqfints
fintsdata_DATA=\
  basic.fints \
  crypt.fints \
  ident.fints \
  prepare.fints \
  dialogend.fints


types:
	for d in $(SUBDIRS); do \
	  $(MAKE) -C $$d types; \
	done

typedefs:
	for d in $(SUBDIRS); do \
	  $(MAKE) -C $$d typedefs; \
	done
