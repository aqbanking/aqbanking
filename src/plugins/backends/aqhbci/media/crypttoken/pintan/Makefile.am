INCLUDES= $(all_includes) -I$(top_srcdir)/src/libs

EXTRA_DIST=pintan.xml.in

noinst_HEADERS=pintan_p.h

plugindir = $(gwenhywfar_plugins)/crypttoken
plugin_LTLIBRARIES=pintan.la
if IS_WINDOWS
plugin_DATA=pintan.xml 
else
plugin_DATA=pintan.xml
endif

pintan_la_LDFLAGS = -no-undefined -module -avoid-version
pintan_la_LIBADD = $(gwenhywfar_libs)
pintan_la_SOURCES=pintan.c

# Add the linker flags for windows resource file
pintan_la_DEPENDENCIES =
# if IS_WINDOWS
if IS_WINDOWS
pintan_la_DEPENDENCIES += ressource.o
pintan_la_LDFLAGS += -Wl,ressource.o
ressource.o: ressource.rc
	$(RC) -o $@ $<
endif
# IS_WINDOWS
# End of linker flags for windows resource file

sources:
	for f in $(pintan_la_SOURCES); do \
	  echo $(subdir)/$$f >>$(top_srcdir)/i18nsources; \
	done
