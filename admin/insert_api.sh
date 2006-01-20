#!/bin/sh

fapi=$1
       

while read line; do
  case "$line" in
    class\ *\ :*)
      line=`echo "$line" | sed "s/class /class $fapi /"`
      ;;
  esac
  echo "$line"
done

