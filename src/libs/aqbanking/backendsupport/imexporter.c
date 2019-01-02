/***************************************************************************
 begin       : Mon Mar 01 2004
 copyright   : (C) 2018 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 * This file is part of the project "AqBanking".                           *
 * Please see toplevel file COPYING of that project for license details.   *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "imexporter_p.h"
#include <gwenhywfar/debug.h>
#include <gwenhywfar/misc.h>
#include <gwenhywfar/inherit.h>
#include <gwenhywfar/syncio_file.h>
#include <gwenhywfar/syncio_memory.h>

#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <ctype.h>


GWEN_INHERIT_FUNCTIONS(AB_IMEXPORTER)
GWEN_LIST_FUNCTIONS(AB_IMEXPORTER, AB_ImExporter)

GWEN_INHERIT(GWEN_PLUGIN, AB_PLUGIN_IMEXPORTER)




AB_IMEXPORTER *AB_ImExporter_new(AB_BANKING *ab, const char *name){
  AB_IMEXPORTER *ie;

  assert(ab);
  assert(name);
  GWEN_NEW_OBJECT(AB_IMEXPORTER, ie);
  GWEN_LIST_INIT(AB_IMEXPORTER, ie);
  GWEN_INHERIT_INIT(AB_IMEXPORTER, ie);

  ie->banking=ab;
  ie->name=strdup(name);

  return ie;
}


void AB_ImExporter_free(AB_IMEXPORTER *ie){
  if (ie) {
    DBG_INFO(AQBANKING_LOGDOMAIN, "Destroying AB_IMEXPORTER");
    GWEN_INHERIT_FINI(AB_IMEXPORTER, ie);
    if (ie->libLoader) {
      GWEN_LibLoader_CloseLibrary(ie->libLoader);
      GWEN_LibLoader_free(ie->libLoader);
    }
    free(ie->name);
    GWEN_LIST_FINI(AB_IMEXPORTER, ie);
    GWEN_FREE_OBJECT(ie);
  }
}



uint32_t AB_ImExporter_GetFlags(const AB_IMEXPORTER *ie) {
  assert(ie);
  return ie->flags;
}



void AB_ImExporter_SetFlags(AB_IMEXPORTER *ie, uint32_t flags) {
  assert(ie);
  ie->flags=flags;
}



void AB_ImExporter_AddFlags(AB_IMEXPORTER *ie, uint32_t flags) {
  assert(ie);
  ie->flags|=flags;
}



void AB_ImExporter_SubFlags(AB_IMEXPORTER *ie, uint32_t flags) {
  assert(ie);
  ie->flags&=~flags;
}



int AB_ImExporter_Import(AB_IMEXPORTER *ie,
                         AB_IMEXPORTER_CONTEXT *ctx,
			 GWEN_SYNCIO *sio,
			 GWEN_DB_NODE *params){
  assert(ie);
  assert(ctx);
  assert(sio);
  assert(params);

  if (ie->importFn) {
    if (GWEN_SyncIo_GetStatus(sio)!=GWEN_SyncIo_Status_Connected) {
      DBG_ERROR(AQBANKING_LOGDOMAIN, "GWEN_SYNCIO %s not connected (%d); did you forget to call GWEN_SyncIo_Connect()?",
		GWEN_SyncIo_GetTypeName(sio),
		GWEN_SyncIo_GetStatus(sio));
      return GWEN_ERROR_NOT_OPEN;
    }

    return ie->importFn(ie, ctx, sio, params);
  }
  else
    return GWEN_ERROR_NOT_SUPPORTED;
}



int AB_ImExporter_Export(AB_IMEXPORTER *ie,
                         AB_IMEXPORTER_CONTEXT *ctx,
			 GWEN_SYNCIO *sio,
			 GWEN_DB_NODE *params){
  assert(ie);
  assert(ctx);
  assert(sio);
  assert(params);

  if (ie->exportFn)
    return ie->exportFn(ie, ctx, sio, params);
  else
    return GWEN_ERROR_NOT_SUPPORTED;
}



int AB_ImExporter_CheckFile(AB_IMEXPORTER *ie,
			    const char *fname){
  assert(ie);
  assert(fname);

  if (ie->checkFileFn)
    return ie->checkFileFn(ie, fname);
  else
    return GWEN_ERROR_NOT_SUPPORTED;
}



int AB_ImExporter_GetEditProfileDialog(AB_IMEXPORTER *ie,
				       GWEN_DB_NODE *dbProfile,
				       const char *testFileName,
				       GWEN_DIALOG **pDlg) {
  assert(ie);
  assert(dbProfile);

  if (ie->getEditProfileDialogFn)
    return ie->getEditProfileDialogFn(ie, dbProfile, testFileName, pDlg);
  else
    return GWEN_ERROR_NOT_SUPPORTED;
}



void AB_ImExporter_SetImportFn(AB_IMEXPORTER *ie,
                               AB_IMEXPORTER_IMPORT_FN f){
  assert(ie);
  ie->importFn=f;
}



void AB_ImExporter_SetExportFn(AB_IMEXPORTER *ie,
                               AB_IMEXPORTER_EXPORT_FN f){
  assert(ie);
  ie->exportFn=f;
}



void AB_ImExporter_SetCheckFileFn(AB_IMEXPORTER *ie,
                                  AB_IMEXPORTER_CHECKFILE_FN f){
  assert(ie);
  ie->checkFileFn=f;
}



void AB_ImExporter_SetGetEditProfileDialogFn(AB_IMEXPORTER *ie,
					     AB_IMEXPORTER_GET_EDITPROFILE_DIALOG_FN f) {
  assert(ie);
  ie->getEditProfileDialogFn=f;
}



AB_BANKING *AB_ImExporter_GetBanking(const AB_IMEXPORTER *ie){
  assert(ie);
  return ie->banking;
}



const char *AB_ImExporter_GetName(const AB_IMEXPORTER *ie){
  assert(ie);
  return ie->name;
}



void AB_ImExporter_SetLibLoader(AB_IMEXPORTER *ie, GWEN_LIBLOADER *ll) {
  assert(ie);
  ie->libLoader=ll;
}










void AB_ImExporter_Utf8ToDta(const char *p,
                             int size,
                             GWEN_BUFFER *buf) {
  while(*p) {
    unsigned int c;

    if (!size)
      break;

    c=(unsigned char)(*(p++));
    if (size!=-1)
      size--;
    switch(c & 0xc0) {
    case 0xc0:
      if (!size) {
        DBG_ERROR(AQBANKING_LOGDOMAIN, "Incomplete UTF-8 sequence");
        c=' ';
        break;
      }
      c=(unsigned char)(*(p++));
      if (size!=-1)
        size--;
      if ((c & 0xc0) != 0x80) {
        DBG_ERROR(AQBANKING_LOGDOMAIN, "Invalid UTF-8 sequence");
        c=' ';
        break;
      }
      if (size && (*p & 0xc0) == 0x80)
        /* a sequence of 3 bytes and more cannot be translated to DTA */
        goto nextUtf8;
      switch(c) {
      case 0x84:
      case 0xa4: c=0x5b; break;
      case 0x96:
      case 0xb6: c=0x5c; break;
      case 0x9c:
      case 0xbc: c=0x5d; break;
      case 0x9f: c=0x7e; break;
      default:   c=' '; break;
      } /* switch */
      break;

    case 0x80:
      DBG_ERROR(AQBANKING_LOGDOMAIN, "Invalid UTF-8 sequence");
    nextUtf8:
      c=' ';
      while(size && (*p & 0xc0) == 0x80) {
        p++;
        if (size!=-1)
          size--;
      }
      break;

    default:
      c=toupper(c);
      if (!(isdigit(c) ||
	    (c>='A' && c<='Z') ||
	    (strchr(" .,&-+*%/$", c))))
        c=' ';
    } /* switch */
    GWEN_Buffer_AppendByte(buf, c);
  } /* while */
}



void AB_ImExporter_DtaToUtf8(const char *p,
                             int size,
                             GWEN_BUFFER *buf) {
  while(*p) {
    unsigned int c;

    if (!size)
      break;

    c=(unsigned char)(*(p++));
    switch(c) {
    case 0x5b: /* AE */
      GWEN_Buffer_AppendByte(buf, 0xc3);
      GWEN_Buffer_AppendByte(buf, 0x84);
      break;

    case 0x5c: /* OE */
      GWEN_Buffer_AppendByte(buf, 0xc3);
      GWEN_Buffer_AppendByte(buf, 0x96);
      break;

    case 0x5d: /* UE */
      GWEN_Buffer_AppendByte(buf, 0xc3);
      GWEN_Buffer_AppendByte(buf, 0x9c);
      break;

    case 0x7e: /* sharp s */
      GWEN_Buffer_AppendByte(buf, 0xc3);
      GWEN_Buffer_AppendByte(buf, 0x9f);
      break;

    default:
      if (c & 0x80) {
        /* produce sane UTF-8 even if something went wrong */
        DBG_ERROR(AQBANKING_LOGDOMAIN, "Invalid character in DTA string");
        c=' ';
      }
      GWEN_Buffer_AppendByte(buf, c);
    }
    if (size!=-1)
      size--;
  } /* while */
}



GWEN_TIME *AB_ImExporter_DateFromString(const char *p, const char *tmpl,
					int inUtc) {
  GWEN_TIME *ti;

  if (strchr(tmpl, 'h')==0) {
    GWEN_BUFFER *dbuf;
    GWEN_BUFFER *tbuf;

    dbuf=GWEN_Buffer_new(0, 32, 0, 1);
    GWEN_Buffer_AppendString(dbuf, p);
    GWEN_Buffer_AppendString(dbuf, "-12:00");

    tbuf=GWEN_Buffer_new(0, 32, 0, 1);
    GWEN_Buffer_AppendString(tbuf, tmpl);
    GWEN_Buffer_AppendString(tbuf, "-hh:mm");

    ti=GWEN_Time_fromUtcString(GWEN_Buffer_GetStart(dbuf),
			       GWEN_Buffer_GetStart(tbuf));
    GWEN_Buffer_free(tbuf);
    GWEN_Buffer_free(dbuf);
  }
  else {
    if (inUtc)
      ti=GWEN_Time_fromUtcString(p, tmpl);
    else
      ti=GWEN_Time_fromString(p, tmpl);
  }
  return ti;
}


void AB_ImExporter_Iso8859_1ToUtf8(const char *p,
                                   int size,
                                   GWEN_BUFFER *buf) {
  while(*p) {
    unsigned int c;

    if (!size)
      break;

    c=(unsigned char)(*(p++));
    if (c<32 || c==127)
      c=32;
    if (c & 0x80) {
      GWEN_Buffer_AppendByte(buf, 0xc0 | c>>6);
      c &= ~0x40;
    }
    GWEN_Buffer_AppendByte(buf, c);
    if (size!=-1)
      size--;
  } /* while */
}



int AB_ImExporter__Transform_Var(GWEN_DB_NODE *db, int level) {
  GWEN_DB_NODE *dbC;

  dbC=GWEN_DB_GetFirstValue(db);
  while(dbC) {
    if (GWEN_DB_GetValueType(dbC)==GWEN_DB_NodeType_ValueChar) {
      const char *s;
      unsigned int l;

      s=GWEN_DB_GetCharValueFromNode(dbC);
      assert(s);
      l=strlen(s);
      if (l) {
        GWEN_BUFFER *vbuf;

        vbuf=GWEN_Buffer_new(0, 1+(l*15/10), 0, 1);
        AB_ImExporter_Iso8859_1ToUtf8(s, l, vbuf);
        GWEN_DB_SetCharValueInNode(dbC, GWEN_Buffer_GetStart(vbuf));
        GWEN_Buffer_free(vbuf);
      }
    }
    dbC=GWEN_DB_GetNextValue(dbC);
  }

  return 0;
}



int AB_ImExporter__Transform_Group(GWEN_DB_NODE *db, int level) {
  GWEN_DB_NODE *dbC;
  int rv;

  if (level>AH_IMEXPORTER_TRANSFORM_MAXLEVEL) {
    DBG_ERROR(AQBANKING_LOGDOMAIN, "DB too deep (%d)", level);
    return -1;
  }

  dbC=GWEN_DB_GetFirstGroup(db);
  while(dbC) {
    rv=AB_ImExporter__Transform_Group(dbC, level+1);
    if (rv)
      return rv;
    dbC=GWEN_DB_GetNextGroup(dbC);
  }

  dbC=GWEN_DB_GetFirstVar(db);
  while(dbC) {
    rv=AB_ImExporter__Transform_Var(dbC, level+1);
    if (rv)
      return rv;
    dbC=GWEN_DB_GetNextVar(dbC);
  }

  return 0;
}



int AB_ImExporter_DbFromIso8859_1ToUtf8(GWEN_DB_NODE *db) {
  return AB_ImExporter__Transform_Group(db, 0);
}









GWEN_PLUGIN *AB_Plugin_ImExporter_new(GWEN_PLUGIN_MANAGER *pm,
				      const char *name,
				      const char *fileName) {
  GWEN_PLUGIN *pl;
  AB_PLUGIN_IMEXPORTER *xpl;

  pl=GWEN_Plugin_new(pm, name, fileName);
  GWEN_NEW_OBJECT(AB_PLUGIN_IMEXPORTER, xpl);
  GWEN_INHERIT_SETDATA(GWEN_PLUGIN, AB_PLUGIN_IMEXPORTER, pl, xpl,
		       AB_Plugin_ImExporter_FreeData);

  return pl;
}



void GWENHYWFAR_CB AB_Plugin_ImExporter_FreeData(void *bp, void *p) {
  AB_PLUGIN_IMEXPORTER *xpl;

  xpl=(AB_PLUGIN_IMEXPORTER*)p;
  GWEN_FREE_OBJECT(xpl);
}



AB_IMEXPORTER *AB_Plugin_ImExporter_Factory(GWEN_PLUGIN *pl,
					    AB_BANKING *ab) {
  AB_PLUGIN_IMEXPORTER *xpl;

  assert(pl);
  xpl=GWEN_INHERIT_GETDATA(GWEN_PLUGIN, AB_PLUGIN_IMEXPORTER, pl);
  assert(xpl);

  assert(xpl->pluginFactoryFn);
  return xpl->pluginFactoryFn(pl, ab);
}


void AB_Plugin_ImExporter_SetFactoryFn(GWEN_PLUGIN *pl,
				       AB_PLUGIN_IMEXPORTER_FACTORY_FN fn) {
  AB_PLUGIN_IMEXPORTER *xpl;

  assert(pl);
  xpl=GWEN_INHERIT_GETDATA(GWEN_PLUGIN, AB_PLUGIN_IMEXPORTER, pl);
  assert(xpl);

  xpl->pluginFactoryFn=fn;
}






