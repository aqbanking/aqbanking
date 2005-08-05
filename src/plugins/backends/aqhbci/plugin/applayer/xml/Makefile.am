

xmlfiles=\
 $(srcdir)/hbciallgrp.xml \
 $(srcdir)/hbci210seg.xml \
 $(srcdir)/hbci210job.xml \
 $(srcdir)/hbci201seg.xml \
 $(srcdir)/hbci201job.xml \
 $(srcdir)/hbci220seg.xml \
 $(srcdir)/hbci220job.xml

EXTRA_DIST=$(xmlfiles)

sources:

base.xml: $(xmlfiles)
	$(XMLMERGE) $(xmlfiles) -v --compact -o $@

all-local: base.xml

CLEANFILES = base.xml
