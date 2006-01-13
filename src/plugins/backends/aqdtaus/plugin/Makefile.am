INCLUDES=-I$(top_builddir)/headers @all_includes@

AM_CFLAGS=-DBUILDING_AQDTAUS @visibility_cflags@

localedir=$(datadir)/locale
DEFS += -DLOCALEDIR=\"$(localedir)\"

EXTRA_DIST=aqdtaus.xml.in

#CLEANFILES =

plugindir=@aqbanking_plugindir@/providers
plugin_LTLIBRARIES=aqdtaus.la
if IS_WINDOWS
plugin_DATA = aqdtaus.xml 
else
plugin_DATA=aqdtaus.xml
endif


lib_LTLIBRARIES=libaqdtaus.la 

noinst_HEADERS=\
 account_l.h \
 account_p.h \
 aqdtaus_l.h \
 job_l.h \
 job_p.h \
 provider_l.h \
 provider_p.h


iheaderdir=${includedir}/aqdtaus
iheader_HEADERS=\
 account.h \
 aqdtaus.h \
 job.h \
 provider.h

libaqdtaus_la_SOURCES= \
 account.c \
 job.c \
 provider.c

aqdtaus_la_SOURCES= \
 aqdtaus-plugin.c

libaqdtaus_la_LIBADD= $(aqbanking_internal_libs) $(gwenhywfar_libs)
aqdtaus_la_LIBADD= \
  libaqdtaus.la \
  $(aqbanking_internal_libs) \
  $(gwenhywfar_libs)
libaqdtaus_la_LDFLAGS = -no-undefined \
  -version-info @AQDTAUS_SO_CURRENT@:@AQDTAUS_SO_REVISION@:@AQDTAUS_SO_AGE@
aqdtaus_la_LDFLAGS = -no-undefined -module \
  -version-info @AQDTAUS_SO_CURRENT@:@AQDTAUS_SO_REVISION@:@AQDTAUS_SO_AGE@

# Add the linker flags for windows resource file
libaqdtaus_la_DEPENDENCIES = $(aqbanking_internal_libs)
aqdtaus_la_DEPENDENCIES = libaqdtaus.la $(aqbanking_internal_libs)
# if IS_WINDOWS
if IS_WINDOWS
libaqdtaus_la_DEPENDENCIES += ressource.o
aqdtaus_la_DEPENDENCIES += ressource.o
libaqdtaus_la_LDFLAGS += -Wl,ressource.o
aqdtaus_la_LDFLAGS += -Wl,ressource.o
ressource.o: ressource.rc
	$(RC) -o $@ $<
endif
# IS_WINDOWS
# End of linker flags for windows resource file

sources:
	for f in $(libaqdtaus_la_SOURCES); do \
	  echo $(subdir)/$$f >>$(top_srcdir)/i18nsources; \
	done
	for d in $(SUBDIRS); do \
	  make -C $$d sources; \
	done

built_sources:

