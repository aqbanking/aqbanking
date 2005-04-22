EXTRA_DIST = types.xml

pkgpython_PYTHON = __init__.py  enum.py  \
 console.py  gtkui.py \
 _basetypes.py \
 _aqtypes.py  

noinst_PYTHON = aqcodegen

xmlsourcedir = $(top_srcdir)/src/libs/aqbanking/types

_aqtypes.py: types.xml $(xmlsourcedir)/*.xml aqcodegen
	./aqcodegen types.xml $(xmlsourcedir) > $@.tmp && \
	  mv $@.tmp $@ || \
	  rm -f $@.tmp
