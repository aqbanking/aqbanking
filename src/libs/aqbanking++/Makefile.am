INCLUDES=-I$(top_builddir)/headers $(gwenhywfar_includes)

lib_LTLIBRARIES=libaqbankingpp.la

#DEFS += -DAQBANKING_PLUGINS=\"$(aqbanking_plugindir)\"

noinst_HEADERS=\
 cppgui_p.h \
 cppgui.h \
 bprogress.h

iheaderdir=${includedir}/aqbanking++
iheader_HEADERS=\
 banking.h \
 cppgui.h

libaqbankingpp_la_SOURCES=\
 banking.cpp \
 cppgui.cpp \
 bprogress.cpp

libaqbankingpp_la_LIBADD = \
 $(aqbanking_internal_libs) \
 $(gwenhywfar_libs) $(i18n_libs)
libaqbankingpp_la_LDFLAGS =


sources:
	for f in $(libaqbankingpp_la_SOURCES); do \
	  echo $(subdir)/$$f >>$(top_srcdir)/i18nsources; \
	done

