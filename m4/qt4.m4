# $Id$
# (c) 2010 Martin Preuss<martin@libchipcard.de>
# These functions search for QT4


AC_DEFUN([AQ_CHECK_QT4],[
dnl PREREQUISITES:
dnl   AQ_CHECK_OS must be called before this
dnl IN: 
dnl   $1 = "yes" if QT4 is needed, "no" if QT4 is optional
dnl   You may preset the return variables.
dnl   All variables which already have a value will not be altered
dnl OUT:
dnl   Variables:
dnl     have_qt4 - set to "yes" if QT4 exists
dnl     qt4_includes - path to includes
dnl     qt4_libs - path to libraries
dnl     qt4_uic - name and path of the uic tool
dnl     qt4_moc - name and path of the moc tool
dnl   Defines:
dnl     HAVE_QT4

lforce="$1"

AC_MSG_CHECKING(if QT4 is allowed)
AC_ARG_ENABLE(qt4,
  [  --enable-qt4         enable qt4 (default=yes)],
  enable_qt4="$enableval",
  enable_qt4="yes")
AC_MSG_RESULT($enable_qt4)

if test "$enable_qt4" = "no"; then
   qt4_libs=""
   qt4_includes=""
   qt4_moc=""
   qt4_uic=""
   have_qt4="no"
else


dnl paths for qt4 includes
AC_MSG_CHECKING(for qt4 includes)
AC_ARG_WITH(qt4-includes, 
  [  --with-qt4-includes=DIR      uses qt4 includes from given dir],
  [local_qt4_includes="$withval"],
  [local_qt4_includes="\
  	$QTDIR/include \
        /usr/include/qt4 \
        /usr/local/include/qt4 \
        /usr/lib/qt4/include \
        /usr/local/lib/qt4/include \
        /opt/qt4/include \
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

if test -z "$qt4_includes"; then
	for i in $local_qt4_includes; do
		if test -z "$qt4_includes"; then
                  if test -f "$i/Qt/qglobal.h"; then
                    lv1=`grep -h "#define QT_VERSION_STR" $i/Qt/qglobal.h`
                    case $lv1 in
                    *4.*)
                  	qt4_includes="-I$i -I$i/Qt -I$i/Qt3Support -I$i/QtCore -I$i/QtGui"
                        qt4_dir=`echo $i | ${SED} 's-/include*--'`
                        break;
                        ;;
                    esac
                  fi
 		fi
        done
fi
if test -n "$qt4_includes"; then
	AC_MSG_RESULT($qt4_includes)
else
	AC_MSG_RESULT(not found)
fi



# Check for x86_64 architecture; potentially set lib-directory suffix
if test "$target_cpu" = "x86_64"; then
  libdirsuffix="64"
else
  libdirsuffix=""
fi

dnl paths for qt4 libs
AC_MSG_CHECKING(for qt4 libraries)
AC_ARG_WITH(qt4-libs, 
  [  --with-qt4-libs=DIR      uses qt4 libs from given dir],
  [local_qt4_libs="$withval"],
  [local_qt4_libs="\
        $qt4_dir/lib${libdirsuffix} \
  	$QTDIR/lib${libdirsuffix} \
        /usr/lib/qt4 \
        /usr/local/lib/qt4 \
        /usr/lib/qt4/lib${libdirsuffix} \
        /usr/local/lib/qt4/lib${libdirsuffix} \
        /opt/qt4/lib${libdirsuffix} \
        /usr/lib/qt \
        /usr/local/lib/qt \
        /usr/lib/qt/lib${libdirsuffix} \
        /usr/local/lib/qt/lib${libdirsuffix} \
        /usr/lib${libdirsuffix} \
        /usr/lib${libdirsuffix}/qt4/lib \
        /usr/lib${libdirsuffix}/qt-4.5/lib \
        /usr/lib${libdirsuffix}/qt-4.6/lib \
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

qt_libname="QtGui"

# This is the name of the qt library to search for.
if test "x$OSYSTEM" = "xdarwin"; then
   qt_searchname="lib${qt_libname}.4.dylib"
else
   qt_searchname="lib${qt_libname}${std_shrext}.4"
fi


if test -z "$qt4_libs"; then
   AQ_SEARCH_FOR_PATH([$qt_searchname],[$local_qt4_libs])
   if test -n "$found_dir" ; then
     qt4_libs="-L$found_dir -l${qt_libname}"
   fi
fi

if test -n "$qt4_libs"; then
	AC_MSG_RESULT($qt4_libs)
else
	AC_MSG_RESULT(not found)
fi


dnl paths for qt4 moc
AC_MSG_CHECKING(for qt4 moc)
if test -z "$qt4_moc"; then
  AC_ARG_WITH(qt4-moc, 
    [  --with-qt4-moc=FILE      uses the given qt4 moc],
    [qt4_moc="$withval"],
    [qt4_moc=""]
  )
fi

if test -z "$qt4_moc"; then
  searchdir="\
    $qt4_dir/bin \
    $QTDIR/bin \
    /usr/lib/qt4/bin \
    /usr/local/lib/qt4/bin \
    /opt/qt4/bin \
    /usr/lib/qt/bin \
    /usr/local/lib/qt/bin \
    /usr/bin \
    /usr/local/bin \
    /opt/qt/bin \
    /usr/X11R6/bin \
    "

# search for "moc-qt4"
  for f in $searchdir; do
    if test -x $f/moc-qt4; then
      qt4_moc="$f/moc-qt4"
      break
    fi
  done

# fall back to "moc"
  if test -z "$qt4_moc"; then
    for f in $searchdir; do
      if test -x $f/moc; then
        qt4_moc="$f/moc"
        break
      fi
    done
  fi
fi

if test -n "$qt4_moc"; then
      AC_MSG_RESULT($qt4_moc)
else
      AC_MSG_RESULT(not found)
fi



dnl paths for qt4 uic
AC_MSG_CHECKING(for qt4 uic)
if test -z "$qt4_uic"; then
  AC_ARG_WITH(qt4-uic, 
    [  --with-qt4-uic=FILE      uses the given qt4 uic],
    [qt4_uic="$withval"],
    [qt4_uic=""]
  )
  
  searchdir="\
    $qt4_dir/bin \
    $QTDIR/bin \
    /usr/lib/qt4/bin \
    /usr/local/lib/qt4/bin \
    /opt/qt4/bin \
    /usr/lib/qt/bin \
    /usr/local/lib/qt/bin \
    /usr/bin \
    /usr/local/bin \
    /opt/qt/bin \
    /usr/X11R6/bin \
    "

# search for "uic-qt4"
  for f in $searchdir; do
    if test -x $f/uic-qt4; then
      qt4_uic="$f/uic-qt4"
      break
    fi
  done

# fall back to "uic"
  if test -z "$qt4_uic"; then
    for f in $searchdir; do
      if test -x $f/uic; then
        qt4_uic="$f/uic"
        break
      fi
    done
  fi
fi

if test -n "$qt4_uic"; then
      AC_MSG_RESULT($qt4_uic)
else
      AC_MSG_RESULT(not found)
fi



# check if all necessary qt4 components where found
if test -z "$qt4_includes" || \
   test -z "$qt4_moc" || \
   test -z "$qt4_uic" || \
   test -z "$qt4_libs"; then
	qt4_libs=""
	qt4_moc=""
	qt4_uic=""
   	qt4_includes=""
   	have_qt4="no"
   	if test "$lforce" = "yes"; then
        	AC_MSG_WARN([
 Compilation of QT applications is enabled but I could not find some QT
 components (see which are missing in messages above).
 If you don't want to compile QT4 applications please use "--disable-qt4".
 ])
   	else
        	AC_MSG_WARN([
 QT4 is not explicitly disabled and I could not find some QT4 components 
 (see which are missing in messages above).
 If you don't want to compile QT4 applications please use "--disable-qt4".
 ])
   	fi
else
dnl TODO: AC_TRY_RUN, check whether qversion.h has matching versions
   have_qt4="yes"
   AC_DEFINE(HAVE_QT4, 1, [whether QT4 is available])
fi


dnl end of if "$enable_qt4"
fi

AS_SCRUB_INCLUDE(qt4_includes)
AC_SUBST(qt4_libs)
AC_SUBST(qt4_includes)
AC_SUBST(qt4_moc)
AC_SUBST(qt4_uic)

])






