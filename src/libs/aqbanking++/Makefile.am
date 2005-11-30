INCLUDES=-I$(top_builddir)/headers $(all_includes)

lib_LTLIBRARIES=libaqbankingpp.la

#DEFS += -DAQBANKING_PLUGINS=\"$(aqbanking_plugindir)\"

noinst_HEADERS=\
 banking_p.h

iheaderdir=$(includedir)/aqbanking++
iheader_HEADERS=\
 banking.h

libaqbankingpp_la_SOURCES=\
 banking.cpp

libaqbankingpp_la_LIBADD = \
 -L$(top_builddir)/src/libs/aqbanking $(aqbanking_internal_libs) \
 $(gwenhywfar_libs)
libaqbankingpp_la_LDFLAGS =

# Add the linker flags for windows resource file
libaqbankingpp_la_DEPENDENCIES = $(aqbanking_internal_libs)
# if IS_WINDOWS
if IS_WINDOWS
libaqbankingpp_la_DEPENDENCIES += ressource.o
libaqbankingpp_la_LDFLAGS += -Wl,ressource.o
ressource.o: ressource.rc
	$(RC) -o $@ $<
endif
# IS_WINDOWS
# End of linker flags for windows resource file

sources:
	for f in $(libaqbankingpp_la_SOURCES); do \
	  echo $(subdir)/$$f >>$(top_srcdir)/i18nsources; \
	done

