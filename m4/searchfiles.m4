# $Id$
# (c) 2002 Martin Preuss<martin@libchipcard.de>
# These functions search for files


AC_DEFUN([AQ_SEARCH_FOR_PATH],[
dnl searches for a file in a path
dnl $1 = file to search
dnl $2 = paths to search in
dnl returns the directory where the file is found (found_dir)
found_dir=""
ls=$1
ld="$2"
for li in $ld; do
    case "$build" in
      *-win32*) fname="$li\\$ls" ;;
      *)        fname="$li/$ls"  ;;
    esac
    
    if test -r "$fname"; then
        found_dir="$li"
        break
    fi
done
])

AC_DEFUN([AQ_SEARCH_FILES],[
dnl searches a dir for some files
dnl $1 = path where to search
dnl $2 = files to find
dnl returns the name of the file found (found_file)
found_file=""
ls="$1"
ld="$2"
lf=""
for li in $ld; do
    lf2="`ls -d ${ls}/${li} 2>/dev/null`"
    lf="$lf $lf2"
done
for li in $lf; do
    if test -r "$li"; then
	found_file=`basename "$li"`
	break
    fi
done
])
