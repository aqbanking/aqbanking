SUBDIRS=prg widgets dialogs views po

INCLUDES = \
  -I$(top_builddir)/headers \
  -I$(srcdir)/dialogs -I$(srcdir)/widgets -I$(srcdir)/prg -I$(srcdir)/views \
  -Idialogs -Iwidgets -Iprg -Iviews \
  $(all_includes)

DEFS += -DDATADIR=\"$(pkgdatadir)\"

lib_LTLIBRARIES=libqbanking.la

bin_SCRIPTS=qbanking-config
M4dir=${prefix}/share/aclocal
dist_M4_DATA=qbanking.m4

iheaderdir=${includedir}/qbanking
iheader_HEADERS=\
 qbflagstaff.h \
 qbanking.h \
 qbwaitcallback.h \
 qbwcb_fast.h \
 qbwcb_simple.h \
 qbwcb_progress.h \
 banking.h

noinst_HEADERS = \
 qbanking_p.h \
 banking_p.h

libqbanking_la_SOURCES=\
 qbflagstaff.cpp \
 qbanking.cpp \
 qbwaitcallback.cpp \
 qbwcb_fast.cpp \
 qbwcb_simple.cpp \
 qbwcb_progress.cpp \
 banking.cpp

nodist_libqbanking_la_SOURCES=\
 qbflagstaff.moc.cpp

qbanking-config: qbanking-config.in
	echo "#! /bin/sh" >qbanking-config && \
	echo 'dir="@prefix@"' >>qbanking-config && \
	cat qbanking-config.in >>qbanking-config && \
	chmod a+x qbanking-config

BUILT_SOURCES =$(nodist_libqbanking_la_SOURCES) version.h
CLEANFILES=$(BUILT_SOURCES) i18nsources qbanking-config qbanking-config.in ressource.rc

libqbanking_la_LIBADD=\
 widgets/libwidgets.la \
 dialogs/libdialogs.la \
 views/libviews.la \
 $(qt3_libs) \
 -L$(top_builddir)/src/libs/aqbanking -laqbanking $(gwenhywfar_libs)

libqbanking_la_LDFLAGS=\
 -version-info @QBANKING_SO_CURRENT@:@QBANKING_SO_REVISION@:@QBANKING_SO_AGE@


sources:
	for f in $(libqbanking_la_SOURCES) $(UI_FILES); do \
	  echo $(subdir)/$$f >>$(top_srcdir)/qtsources; \
	done
	for d in $(SUBDIRS); do \
	  make -C $$d sources; \
	done ;

%.moc.cpp: %.h
	@qt3_moc@ -o $@ $<


clean-local:
	rm -f *.ui.{h.cpp} *.moc.* >/dev/null

clean-ui: 
	rm -f *.ui.{cpp,h}


# This section is for a qt4 environment
qt4-port:
	for A in $(libqbanking_la_SOURCES) $(HEADERS); do \
	 $(YES) | $(QT3TO4) $$A; \
	done
	for d in $(SUBDIRS); do \
	  make -C $$d qt4-port; \
	done
# For qt3 compatibility of qt4
DEFS += -DQT3_SUPPORT
