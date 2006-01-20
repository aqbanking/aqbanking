#!/bin/sh

# -------------------------------------------------------------------------
# This tool inserts export declarations into declarations of classes. e.g.
#    class MYCLASS : SOMECLASS
#    { lalala
#    ...
# would become
#    class QBANKING_API MYCLASS : SOMECLASS
#    { lalala
#    ...
# when used for QBanking.
# The first (and only) argument is the export declaration to insert.
# It reads from stdin and writes to stdout.
#
# (c) 2006 Martin Preuss
#


fapi=$1
       

while read line; do
  case "$line" in
    class\ *\ :*)
      line=`echo "$line" | sed "s/class /class $fapi /"`
      ;;
  esac
  echo "$line"
done

