# $Id$
# (c) 2010 Martin Preuss<martin@libchipcard.de>
# These functions search for QT3


AC_DEFUN([AQ_CHECK_QT3],[
dnl PREREQUISITES:
dnl   AQ_CHECK_OS must be called before this
dnl IN: 
dnl   $1 = "yes" if QT3 is needed, "no" if QT3 is optional
dnl   You may preset the return variables.
dnl   All variables which already have a value will not be altered
dnl OUT:
dnl   Variables:
dnl     have_qt3 - set to "yes" if QT3 exists
dnl     qt3_includes - path to includes
dnl     qt3_libs - path to libraries
dnl     qt3_uic - name and path of the uic tool
dnl     qt3_moc - name and path of the moc tool
dnl   Defines:
dnl     HAVE_QT3

lforce="$1"

AC_MSG_CHECKING(if QT3 is allowed)
AC_ARG_ENABLE(qt3,
  [  --enable-qt3         enable qt3 (default=yes)],
  enable_qt3="$enableval",
  enable_qt3="yes")
AC_MSG_RESULT($enable_qt3)

if test "$enable_qt3" = "no"; then
   qt3_libs=""
   qt3_includes=""
   qt3_moc=""
   qt3_uic=""
   have_qt3="no"
else


dnl paths for qt3 includes
AC_MSG_CHECKING(for qt3 includes)
AC_ARG_WITH(qt3-includes, 
  [  --with-qt3-includes=DIR      uses qt3 includes from given dir],
  [local_qt3_includes="$withval"],
  [local_qt3_includes="\
  	$QTDIR/include \
        /usr/include/qt3 \
        /usr/local/include/qt3 \
        /usr/lib/qt3/include \
        /usr/local/lib/qt3/include \
        /opt/qt3/include \
        /usr/include/qt \
        /usr/local/include/qt \
        /usr/lib/qt/include \
        /usr/local/lib/qt/include \
        /usr/include \
        /usr/local/include \
        /opt/qt/include \
        /usr/X11R6/include \
        "
  ]
)

if test -z "$qt3_includes"; then
	for i in $local_qt3_includes; do
		if test -z "$qt3_includes"; then
                  if test -f "$i/qglobal.h"; then
                    lv1=`grep -h "#define QT_VERSION_STR" $i/qglobal.h`
                    case $lv1 in
                    *3.*)
                  	qt3_includes="-I$i"
                        qt3_dir=`echo $i | ${SED} 's-/include*--'`
                        break;
                        ;;
                    esac
                  fi
 		fi
        done
fi
if test -n "$qt3_includes"; then
	AC_MSG_RESULT($qt3_includes)
else
	AC_MSG_RESULT(not found)
fi



AC_MSG_CHECKING(if threaded qt3 may be used)
AC_ARG_ENABLE(qt3-threads,
  [  --enable-qt3-threads         enable qt3-mt library (default=yes)],
  enable_qt3_threads="$enableval",
  enable_qt3_threads="yes")
AC_MSG_RESULT($enable_qt3_threads)



# Check for x86_64 architecture; potentially set lib-directory suffix
if test "$target_cpu" = "x86_64"; then
  libdirsuffix="64"
else
  libdirsuffix=""
fi

dnl paths for qt3 libs
AC_MSG_CHECKING(for qt3 libraries)
AC_ARG_WITH(qt3-libs, 
  [  --with-qt3-libs=DIR      uses qt3 libs from given dir],
  [local_qt3_libs="$withval"],
  [local_qt3_libs="\
  	$qt3_dir/lib${libdirsuffix} \
  	$QTDIR/lib${libdirsuffix} \
        /usr/lib/qt3 \
        /usr/local/lib/qt3 \
        /usr/lib/qt3/lib${libdirsuffix} \
        /usr/local/lib/qt3/lib${libdirsuffix} \
        /opt/qt3/lib${libdirsuffix} \
        /usr/lib/qt \
        /usr/local/lib/qt \
        /usr/lib/qt/lib${libdirsuffix} \
        /usr/local/lib/qt/lib${libdirsuffix} \
        /usr/lib${libdirsuffix} \
        /usr/lib${libdirsuffix}/qt3/lib \
        /usr/lib${libdirsuffix}/qt-3.3/lib \
        /usr/lib${libdirsuffix}/qt-3.2/lib \
        /usr/local/lib${libdirsuffix} \
        /opt/qt/lib${libdirsuffix} \
        /usr/X11R6/lib${libdirsuffix} \
        "
  ]
)

# Determine the extension of a shared library; the variable
# std_shrext comes from the AC_PROG_LIBTOOL macro. Copied from
# libtool.

# Shared library suffix. On linux this was set as
# shrext_cmds='.so'; but on darwin it is actually a text command.
eval std_shrext=\"$shrext_cmds\"
if test -n "${std_shrext}"; then
   std_shrext='.so'
fi

# Choose library name of qt
if test "$enable_qt3_threads" != "no"; then
   qt_libname="qt-mt"
else
   qt_libname="qt"
fi

# This is the name of the qt library to search for.
if test "x$OSYSTEM" = "xdarwin"; then
   qt_searchname="lib${qt_libname}.3.dylib"
else
   qt_searchname="lib${qt_libname}${std_shrext}.3"
fi


if test -z "$qt3_libs"; then
   AQ_SEARCH_FOR_PATH([$qt_searchname],[$local_qt3_libs])
   if test -n "$found_dir" ; then
     qt3_libs="-L$found_dir -l${qt_libname}"
   fi
fi

if test -n "$qt3_libs"; then
	AC_MSG_RESULT($qt3_libs)
else
	AC_MSG_RESULT(not found)
fi


dnl paths for qt3 moc
AC_MSG_CHECKING(for qt3 moc)
if test -z "$qt3_moc"; then
  AC_ARG_WITH(qt3-moc, 
    [  --with-qt3-moc=FILE      uses the given qt3 moc],
    [qt3_moc="$withval"],
    [qt3_moc=""]
  )
fi

if test -z "$qt3_moc"; then
  searchdir="\
    $qt3_dir/bin \
    $QTDIR/bin \
    /usr/lib/qt3/bin \
    /usr/local/lib/qt3/bin \
    /opt/qt3/bin \
    /usr/lib/qt/bin \
    /usr/local/lib/qt/bin \
    /usr/bin \
    /usr/local/bin \
    /opt/qt/bin \
    /usr/X11R6/bin \
    "

# search for "moc-qt3"
  for f in $searchdir; do
    if test -x $f/moc-qt3; then
      qt3_moc="$f/moc-qt3"
      break
    fi
  done

# fall back to "moc"
  if test -z "$qt3_moc"; then
    for f in $searchdir; do
      if test -x $f/moc; then
        qt3_moc="$f/moc"
        break
      fi
    done
  fi

fi

if test -n "$qt3_moc"; then
      AC_MSG_RESULT($qt3_moc)
else
      AC_MSG_RESULT(not found)
fi



dnl paths for qt3 uic
AC_MSG_CHECKING(for qt3 uic)
if test -z "$qt3_uic"; then
  AC_ARG_WITH(qt3-uic, 
    [  --with-qt3-uic=FILE      uses the given qt3 uic],
    [qt3_uic="$withval"],
    [qt3_uic=""]
  )
  
  searchdir="\
    $qt3_dir/bin \
    $QTDIR/bin \
    /usr/lib/qt3/bin \
    /usr/local/lib/qt3/bin \
    /opt/qt3/bin \
    /usr/lib/qt/bin \
    /usr/local/lib/qt/bin \
    /usr/bin \
    /usr/local/bin \
    /opt/qt/bin \
    /usr/X11R6/bin \
    "

# search for "uic-qt3"
  for f in $searchdir; do
    if test -x $f/uic-qt3; then
      qt3_uic="$f/uic-qt3"
      break
    fi
  done

# fall back to "uic"
  if test -z "$qt3_uic"; then
    for f in $searchdir; do
      if test -x $f/uic; then
        qt3_uic="$f/uic"
        break
      fi
    done
  fi
fi

if test -n "$qt3_uic"; then
      AC_MSG_RESULT($qt3_uic)
else
      AC_MSG_RESULT(not found)
fi



# check if all necessary qt3 components where found
if test -z "$qt3_includes" || \
   test -z "$qt3_moc" || \
   test -z "$qt3_uic" || \
   test -z "$qt3_libs"; then
	qt3_libs=""
	qt3_moc=""
	qt3_uic=""
   	qt3_includes=""
   	have_qt3="no"
   	if test "$lforce" = "yes"; then
        	AC_MSG_WARN([
 Compilation of QT applications is enabled but I could not find some QT
 components (see which are missing in messages above).
 If you don't want to compile QT3 applications please use "--disable-qt3".
 ])
   	else
        	AC_MSG_WARN([
 QT3 is not explicitly disabled and I could not find some QT3 components 
 (see which are missing in messages above).
 If you don't want to compile QT3 applications please use "--disable-qt3".
 ])
   	fi
else
dnl TODO: AC_TRY_RUN, check whether qversion.h has matching versions
   have_qt3="yes"
   AC_DEFINE(HAVE_QT3, 1, [whether QT3 is available])
fi


dnl end of if "$enable_qt3"
fi

AS_SCRUB_INCLUDE(qt3_includes)
AC_SUBST(qt3_libs)
AC_SUBST(qt3_includes)
AC_SUBST(qt3_moc)
AC_SUBST(qt3_uic)

])






