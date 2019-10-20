
fintsdatadir=$(aqbanking_pkgdatadir)/backends/aqfints
fintsdata_DATA=\
  basic.fints \
  crypt.fints \
  ident.fints \
  prepare.fints \
  dialogend.fints


typefiles:
	for d in $(SUBDIRS); do \
	  $(MAKE) -C $$d typefiles; \
	done

typedefs:
	for d in $(SUBDIRS); do \
	  $(MAKE) -C $$d typedefs; \
	done
