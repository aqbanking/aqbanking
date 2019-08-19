
INCLUDES = -I$(top_srcdir)/src/libs -I$(top_builddir)/src/libs $(gwenhywfar_includes) -I$(srcdir)/../.. $(AQEBICS_CFLAGS)

#EXTRA_DIST=

localedir = $(datadir)/locale
DEFS += -DLOCALEDIR=\"$(localedir)\"
AM_CFLAGS=-DBUILDING_AQEBICS @visibility_cflags@


noinst_LTLIBRARIES=libebicscontrol.la


noinst_HEADERS=globals.h control_l.h

libebicscontrol_la_SOURCES=main.c \
 addaccount.c \
 addsubuserflags.c \
 adduser.c \
 createkeys.c \
 createtempkey.c \
 download.c \
 sendkeys.c \
 sendsignkey.c \
 getaccounts.c \
 getkeys.c \
 iniletter.c \
 hialetter.c \
 sendhpd.c \
 sendhkd.c \
 upload.c \
 utils.c \
 mkpinlist.c \
 resetuser.c \
 getcert.c \
 listusers.c \
 listaccounts.c \
 setebicsversion.c


sources:
	for d in $(SUBDIRS); do \
	  $(MAKE) -C $$d sources; \
	done
	for f in $(libebicscontrol_la_SOURCES); do \
	  echo $(subdir)/$$f >>$(top_srcdir)/i18nsources; \
	done


cppcheck:
	for f in $(libebicscontrol_la_SOURCES); do \
	  cppcheck --force $$f ; \
	done
	for d in $(SUBDIRS); do \
	  $(MAKE) -C $$d cppcheck; \
	done


