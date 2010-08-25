/***************************************************************************
    begin       : Mon Mar 01 2004
    copyright   : (C) 2004 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif


#include "eri2_p.h"
#include <gwenhywfar/debug.h>
#include <gwenhywfar/text.h>
#include <gwenhywfar/gui.h>
#include <gwenhywfar/fastbuffer.h>
#include <gwenhywfar/directory.h>
#include <gwenhywfar/syncio_file.h>
#include <gwenhywfar/syncio_buffered.h>

#include <aqbanking/banking_be.h>
#include <aqbanking/msgengine.h>
#include <aqbanking/banking_be.h>
#include "i18n_l.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <assert.h>
#include <ctype.h>


#ifdef OS_WIN32
# define DIRSEP "\\"
#else
# define DIRSEP "/"
#endif

#define AB_IMEXPORTER_ERI2_CHECKBUF_LENGTH 128


GWEN_INHERIT(AB_IMEXPORTER, AB_IMEXPORTER_ERI2)



GWEN_PLUGIN *imexporter_eri2_factory(GWEN_PLUGIN_MANAGER *pm,
				     const char *name,
				     const char *fileName) {
  GWEN_PLUGIN *pl;

  pl=AB_Plugin_ImExporter_new(pm, name, fileName);
  assert(pl);

  AB_Plugin_ImExporter_SetFactoryFn(pl, AB_Plugin_ImExporterERI2_Factory);

  return pl;
}



AB_IMEXPORTER *AB_Plugin_ImExporterERI2_Factory(GWEN_PLUGIN *pl,
						AB_BANKING *ab){
  AB_IMEXPORTER *ie;
  AB_IMEXPORTER_ERI2 *ieh;
  GWEN_STRINGLIST *paths;

  ie=AB_ImExporter_new(ab, "eri2");
  GWEN_NEW_OBJECT(AB_IMEXPORTER_ERI2, ieh);
  GWEN_INHERIT_SETDATA(AB_IMEXPORTER, AB_IMEXPORTER_ERI2, ie, ieh,
		       AB_ImExporterERI2_FreeData);
  paths=AB_Banking_GetGlobalDataDirs();
  if (paths) {
    GWEN_BUFFER *fbuf;
    int rv;

    fbuf=GWEN_Buffer_new(0, 256, 0, 1);
    rv=GWEN_Directory_FindFileInPaths(paths,
				      "aqbanking"
				      DIRSEP
				      "imexporters"
				      DIRSEP
				      "eri2"
				      DIRSEP
				      AB_ERI2_XMLFILE,
				      fbuf);
    GWEN_StringList_free(paths);
    if (rv) {
      DBG_ERROR(AQBANKING_LOGDOMAIN, "XML data file not found (%d)", rv);
      GWEN_Buffer_free(fbuf);
      return NULL;
    }
    else {
      GWEN_XMLNODE *xmlNode;

      xmlNode=GWEN_XMLNode_new(GWEN_XMLNodeTypeTag, "root");

      rv=GWEN_XML_ReadFile(xmlNode,
			   GWEN_Buffer_GetStart(fbuf),
			   GWEN_XML_FLAGS_DEFAULT |
			   GWEN_XML_FLAGS_HANDLE_HEADERS);
      if (rv) {
	DBG_ERROR(AQBANKING_LOGDOMAIN, "Could not load XML file [%s]: %d.\n",
		  GWEN_Buffer_GetStart(fbuf), rv);
	GWEN_XMLNode_free(xmlNode);
	GWEN_Buffer_free(fbuf);
	return NULL;
      }
      GWEN_Buffer_free(fbuf);

      ieh->msgEngine = AB_MsgEngine_new();
      GWEN_MsgEngine_SetDefinitions(ieh->msgEngine, xmlNode, 1);

      AB_ImExporter_SetImportFn(ie, AB_ImExporterERI2_Import);
      AB_ImExporter_SetExportFn(ie, AB_ImExporterERI2_Export);
      AB_ImExporter_SetCheckFileFn(ie, AB_ImExporterERI2_CheckFile);
      return ie;
    }
  }
  else {
    DBG_ERROR(AQBANKING_LOGDOMAIN, "No data files found.");
    AB_ImExporter_free(ie);
    return NULL;
  }
}



void GWENHYWFAR_CB AB_ImExporterERI2_FreeData(void *bp, void *p){
  AB_IMEXPORTER_ERI2 *ieh;

  ieh=(AB_IMEXPORTER_ERI2*)p;
  GWEN_FREE_OBJECT(ieh);
}



int AB_ImExporterERI2_Import(AB_IMEXPORTER *ie,
                             AB_IMEXPORTER_CONTEXT *ctx,
			     GWEN_SYNCIO *sio,
			     GWEN_DB_NODE *params){
  AB_IMEXPORTER_ERI2 *ieh;
  GWEN_DB_NODE *dbData;
  int rv;
  GWEN_BUFFER *mbuf;
  GWEN_FAST_BUFFER *fb;

  assert(ie);
  ieh = GWEN_INHERIT_GETDATA(AB_IMEXPORTER, AB_IMEXPORTER_ERI2, ie);
  assert(ieh);

  mbuf = GWEN_Buffer_new(0, 1024, 0, 1);
  dbData = GWEN_DB_Group_new("transactions");

  fb=GWEN_FastBuffer_new(512, sio);

  /* parse into db */
  for (;;) {
    int rv;
    int c;

    GWEN_Buffer_Reset(mbuf);
    GWEN_FASTBUFFER_PEEKBYTE(fb, c);
    if (c==GWEN_ERROR_EOF)
      break;
    else if (c==26) {
      GWEN_FASTBUFFER_READBYTE(fb, c);
      break;
    }
    else if (c<0) {
      DBG_ERROR(0, "Error reading message");
      GWEN_FastBuffer_free(fb);
      GWEN_Buffer_free(mbuf);
      GWEN_DB_Group_free(dbData);
      return c;
    }

    rv=GWEN_FastBuffer_ReadLineToBuffer(fb, mbuf);
    if (rv<0) {
      DBG_INFO(AQBANKING_LOGDOMAIN, "here (%d)", rv);
      GWEN_FastBuffer_free(fb);
      GWEN_Buffer_free(mbuf);
      GWEN_DB_Group_free(dbData);
      return rv;
    }

    GWEN_Buffer_Rewind(mbuf);

    rv = GWEN_MsgEngine_ReadMessage(ieh->msgEngine, "SEG", mbuf, dbData, 0);
    if (rv) {
      GWEN_FastBuffer_free(fb);
      GWEN_Buffer_free(mbuf);
      GWEN_DB_Group_free(dbData);
      return GWEN_ERROR_GENERIC;
    }
  }
  GWEN_FastBuffer_free(fb);
  GWEN_Buffer_free(mbuf);

  /* import from db */
  GWEN_Gui_ProgressLog(0, GWEN_LoggerLevel_Notice,
		       I18N("Data imported, transforming to UTF-8"));
  rv=AB_ImExporter_DbFromIso8859_1ToUtf8(dbData);
  if (rv) {
    GWEN_Gui_ProgressLog(0, GWEN_LoggerLevel_Error,
			 "Error converting data");
    GWEN_DB_Group_free(dbData);
    return rv;
  }
  GWEN_Gui_ProgressLog(0, GWEN_LoggerLevel_Notice,
		       "Transforming data to transactions");
  rv = AB_ImExporterERI2__ImportFromGroup(ctx, dbData, params);
  if (rv) {
    DBG_INFO(AQBANKING_LOGDOMAIN, "here (%d)", rv);
    GWEN_DB_Group_free(dbData);
    return rv;
  }
  GWEN_DB_Group_free(dbData);

  return 0;
}



const char* AB_ImExporterERI2__StripPZero(const char *p) {

  while ((*p == '0') || (*p == 'P')) p++;
  return p;

}



int AB_ImExporterERI2__HandleRec1(GWEN_DB_NODE *dbT,
                                  GWEN_DB_NODE *dbParams,
                                  AB_TRANSACTION *t) {
  const char *p;
  const char *dateFormat;
  int inUtc;

  dateFormat = GWEN_DB_GetCharValue(dbParams, "dateFormat", 0, "YYMMDD");
  inUtc = GWEN_DB_GetIntValue(dbParams, "utc", 0, 0);

  /* strip leading zeroes from localaccountnumber
     can be removed when lfiller="48" does what I expect from i */
  p = GWEN_DB_GetCharValue(dbT, "localAccountNumber", 0, 0);
  p = AB_ImExporterERI2__StripPZero(p);
  AB_Transaction_SetLocalAccountNumber(t, p);

  /* strip leading P and zeroes from remoteaccountnumber
     this CANNOT be done with lfiller="48" becaus of the P added
     to Postgiro accounts */
  p = GWEN_DB_GetCharValue(dbT, "remoteAccountNumber", 0, 0);
  p = AB_ImExporterERI2__StripPZero(p);

#ifdef ERI2DEBUG
  printf("Remote Account Number after StripPZero is %s\n", p);
#endif

  AB_Transaction_SetRemoteAccountNumber(t, p);

  /* translate value */
  p = GWEN_DB_GetCharValue(dbT, "Amount", 0, 0);
  if (p) {
    AB_VALUE *v;
    AB_VALUE *v2;

    /* divide by 100 */
    v=AB_Value_fromString(p);
    v2=AB_Value_fromDouble(100.0);
    AB_Value_DivValue(v, v2);
    AB_Value_free(v2);

    AB_Value_SetCurrency(v, GWEN_DB_GetCharValue(dbT, "currency", 0, "EUR"));
    AB_Transaction_SetValue(t, v);
    AB_Value_free(v);
  }
  
  /* translate date */
  p = GWEN_DB_GetCharValue(dbT, "date", 0, 0);
  if (p) {
    GWEN_TIME *ti;

    ti = AB_ImExporter_DateFromString(p, dateFormat, inUtc);
    if (ti)
      AB_Transaction_SetDate(t, ti);
    GWEN_Time_free(ti);
  }
  
  /* translate valutaDate */
  p = GWEN_DB_GetCharValue(dbT, "valutaDate", 0, 0);
  if (p) {
    GWEN_TIME *ti;
  
    ti = AB_ImExporter_DateFromString(p, dateFormat, inUtc);
    if (ti)
      AB_Transaction_SetValutaDate(t, ti);
    GWEN_Time_free(ti);
  }
  
  /* possibly translate value */
  p = GWEN_DB_GetCharValue(dbT, "Sign", 0, 0);
  if (p) {
    int determined=0;
    int j;
  
    /* get positive/negative mark */
  
    /* try positive marks first */
    for (j=0; ; j++) {
      const char *patt;
  
      patt = GWEN_DB_GetCharValue(dbParams, "positiveValues", j, 0);
      if (!patt) {
        if (j == 0)
          patt = "C";
        else
          break;
      }
      if (-1 != GWEN_Text_ComparePattern(p, patt, 0)) {
        /* value already is positive, keep it that way */
        determined = 1;
        break;
      }
    } /* for */
  
    if (!determined) {
      for (j=0; ; j++) {
        const char *patt;
  
        patt = GWEN_DB_GetCharValue(dbParams, "negativeValues", j, 0);
        if (!patt) {
          if (j == 0)
            patt = "D";
          else
            break;
        }
        if (-1 != GWEN_Text_ComparePattern(p, patt, 0)) {
          const AB_VALUE *pv;
  
          /* value must be negated */
          pv = AB_Transaction_GetValue(t);
          if (pv) {
            AB_VALUE *v;
  
            v = AB_Value_dup(pv);
            AB_Value_Negate(v);
            AB_Transaction_SetValue(t, v);
            AB_Value_free(v);
          }
          determined = 1;
          break;
        }
      } /* for */
    }
  } /* if sign mark */

  return 0;
}



void AB_ImExporterERI2__AddPurpose(AB_TRANSACTION *t, const char *s) {

  if (strlen(s) > 0)
    AB_Transaction_AddPurpose(t, s, 0);

}



int AB_ImExporterERI2__HandleRec2(GWEN_DB_NODE *dbT,
                                  GWEN_DB_NODE *dbParams,
                                  AB_TRANSACTION *t) {
  const char *p;

  p = GWEN_DB_GetCharValue(dbT, "purpose1", 0, 0);
  if (p)
  AB_ImExporterERI2__AddPurpose(t, p);

  p = GWEN_DB_GetCharValue(dbT, "purpose2", 0, 0);
  if (p)
  AB_ImExporterERI2__AddPurpose(t, p);

  return 0;
}



int AB_ImExporterERI2__HandleRec3(GWEN_DB_NODE *dbT,
                                  GWEN_DB_NODE *dbParams,
                                  AB_TRANSACTION *t) {
  const char *p;

  p = GWEN_DB_GetCharValue(dbT, "purpose3", 0, 0);
  if (p)
  AB_ImExporterERI2__AddPurpose(t, p);

  p = GWEN_DB_GetCharValue(dbT, "purpose4", 0, 0);
  if (p)
  AB_ImExporterERI2__AddPurpose(t, p);

  p = GWEN_DB_GetCharValue(dbT, "purpose5", 0, 0);
  if (p)
  AB_ImExporterERI2__AddPurpose(t, p);

  return 0;
}



int AB_ImExporterERI2__HandleRec4(GWEN_DB_NODE *dbT,
				  GWEN_DB_NODE *dbParams,
				  AB_TRANSACTION *t) {

  const char *p1, *p2, *p3;
  GWEN_BUFFER *pbuf;
  char strbuf[97];
  unsigned int strlen = 0, *ps = &strlen;

  pbuf = GWEN_Buffer_new(0, 96, 0, 1);

  p1 = GWEN_DB_GetCharValue(dbT, "purpose3", 0, 0);
  p2 = GWEN_DB_GetCharValue(dbT, "purpose4", 0, 0);
  p3 = GWEN_DB_GetCharValue(dbT, "purpose5", 0, 0);

  if (p1) GWEN_Buffer_AppendString(pbuf, p1);
  if (GWEN_Buffer_GetUsedBytes(pbuf) < 32)
    GWEN_Buffer_AppendString(pbuf, " ");
  if (p2)
    GWEN_Buffer_AppendString(pbuf, p2);
  if (GWEN_Buffer_GetUsedBytes(pbuf) < 64)
    GWEN_Buffer_AppendString(pbuf, " ");
  if (p3)
    GWEN_Buffer_AppendString(pbuf, p3);

  strlen = GWEN_Buffer_GetUsedBytes(pbuf);
  if (strlen) {
    GWEN_Buffer_Rewind(pbuf);
    GWEN_Buffer_ReadBytes(pbuf, strbuf, ps);
    strbuf[strlen] = 0;
    AB_ImExporterERI2__AddPurpose(t, strbuf);
  }
  GWEN_Buffer_free(pbuf);
  return 0;
}



void AB_ImExporterERI2__AddTransaction(AB_IMEXPORTER_CONTEXT *ctx,
                                       AB_TRANSACTION *t,
                                       GWEN_DB_NODE *params) {
  AB_IMEXPORTER_ACCOUNTINFO *iea = 0;
  const char *bankName;
  const char *la;

  bankName = GWEN_DB_GetCharValue(params, "bankName", 0, "Rabobank");

  /* Search if account number is already in context
     If so add transaction there, else make new account number in context. */
  iea = AB_ImExporterContext_GetFirstAccountInfo(ctx);
  la = AB_Transaction_GetLocalAccountNumber(t);
  assert(la);
  while(iea) {
    if (strcmp(AB_ImExporterAccountInfo_GetAccountNumber(iea),
               AB_Transaction_GetLocalAccountNumber(t))==0)
      break;
    iea = AB_ImExporterContext_GetNextAccountInfo(ctx);
  }

  if (!iea) {
    /* Not found, add it */
    iea = AB_ImExporterAccountInfo_new();
    AB_ImExporterContext_AddAccountInfo(ctx, iea);
    AB_ImExporterAccountInfo_SetType(iea, AB_AccountType_Bank);
    AB_ImExporterAccountInfo_SetBankName(iea, bankName);
    AB_ImExporterAccountInfo_SetAccountNumber(iea, la);
  }

  /* Add it to the AccountInfo List */
  AB_ImExporterAccountInfo_AddTransaction(iea, t);
}



int AB_ImExporterERI2__ImportFromGroup(AB_IMEXPORTER_CONTEXT *ctx,
                                       GWEN_DB_NODE *db,
				       GWEN_DB_NODE *dbParams) {
  GWEN_DB_NODE *dbT;

  dbT = GWEN_DB_FindFirstGroup(db, "RecordType1");
  while(dbT) {
    if (GWEN_DB_GetCharValue(dbT, "amount", 0, 0)) {
      AB_TRANSACTION *t;
      GWEN_DB_NODE *dbN;
      int rv;

      DBG_DEBUG(AQBANKING_LOGDOMAIN, "Found a possible transaction");
      t = AB_Transaction_fromDb(dbT);
      if (!t) {
        DBG_ERROR(AQBANKING_LOGDOMAIN, "Error in config file");
        GWEN_Gui_ProgressLog(0, GWEN_LoggerLevel_Error,
			     "Error in config file");
        return GWEN_ERROR_GENERIC;
      }

      rv = AB_ImExporterERI2__HandleRec1(dbT, dbParams, t);
      if (rv) {
        DBG_INFO(AQBANKING_LOGDOMAIN, "here (%d)", rv);
        AB_Transaction_free(t);
        return rv;
      }

      /* check whether the next group is of record type 2 */
      dbN = GWEN_DB_GetNextGroup(dbT);
      if (dbN) {
        if (strcasecmp(GWEN_DB_GroupName(dbN), "RecordType2") == 0) {
          int num3;
          int i;

          rv = AB_ImExporterERI2__HandleRec2(dbN, dbParams, t);
          if (rv) {
            DBG_INFO(AQBANKING_LOGDOMAIN, "here (%d)", rv);
            AB_Transaction_free(t);
            return rv;
          }
          num3 = GWEN_DB_GetIntValue(dbN, "NumberOfExtraRecords", 0, 0);
          for (i = 0; i < num3; i++) {
            dbN = GWEN_DB_GetNextGroup(dbN);
            if (!dbN)
              break;
            else {
              if (strcasecmp(GWEN_DB_GroupName(dbN), "RecordType1") == 0)
                break;
              if (strcasecmp(GWEN_DB_GroupName(dbN), "RecordType3") == 0) {
                if (!i) {
		  rv = AB_ImExporterERI2__HandleRec3(dbN, dbParams, t);
		} else {
		  rv = AB_ImExporterERI2__HandleRec4(dbN, dbParams, t);
		}
                if (rv) {
                  DBG_INFO(AQBANKING_LOGDOMAIN, "here (%d)", rv);
                  AB_Transaction_free(t);
                  return rv;
                }
              }
            }
          } /* for */
          if (i != num3) {
            DBG_ERROR(AQBANKING_LOGDOMAIN,
                      "Missing records (have %d of %d)", i, num3);
            AB_Transaction_free(t);
            return rv;
          }
        } /* if type 2 follows */
      } /* if any group follows */

      DBG_NOTICE(AQBANKING_LOGDOMAIN, "Adding transaction");
      AB_ImExporterERI2__AddTransaction(ctx, t, dbParams);
    }
    else {
      DBG_ERROR(AQBANKING_LOGDOMAIN, "Empty group");
      GWEN_DB_Dump(dbT, 2);
    }

    dbT=GWEN_DB_FindNextGroup(dbT, "RecordType1");
  } // while

  return 0;
}



int AB_ImExporterERI2_CheckFile(AB_IMEXPORTER *ie, const char *fname){
  GWEN_BUFFER *lbuffer;
  AB_IMEXPORTER_ERI2 *ieh;
  GWEN_SYNCIO *sio;
  GWEN_SYNCIO *baseIo;
  int rv;

  assert(ie);
  ieh=GWEN_INHERIT_GETDATA(AB_IMEXPORTER, AB_IMEXPORTER_ERI2, ie);
  assert(ieh);

  assert(fname);

  sio=GWEN_SyncIo_File_new(fname, GWEN_SyncIo_File_CreationMode_OpenExisting);
  GWEN_SyncIo_AddFlags(sio, GWEN_SYNCIO_FILE_FLAGS_READ);
  baseIo=sio;

  sio=GWEN_SyncIo_Buffered_new(baseIo);
  rv=GWEN_SyncIo_Connect(sio);
  if (rv<0) {
    /* error */
    DBG_ERROR(AQBANKING_LOGDOMAIN,
	      "open(%s): %s", fname, strerror(errno));
    return GWEN_ERROR_IO;
  }

  lbuffer=GWEN_Buffer_new(0, 256, 0, 1);
  rv=GWEN_SyncIo_Buffered_ReadLineToBuffer(sio, lbuffer);
  if (rv<0) {
    DBG_INFO(AQBANKING_LOGDOMAIN,
	     "File \"%s\" is not supported by this plugin",
	     fname);
    GWEN_Buffer_free(lbuffer);
    GWEN_SyncIo_Disconnect(sio);
    GWEN_SyncIo_free(sio);
    return GWEN_ERROR_BAD_DATA;
  }

  if ( -1 != GWEN_Text_ComparePattern(GWEN_Buffer_GetStart(lbuffer), "*EUR99999999992000*", 0)) {
    /* match */
    DBG_INFO(AQBANKING_LOGDOMAIN,
	     "File \"%s\" is supported by this plugin",
	     fname);
    GWEN_Buffer_free(lbuffer);
    GWEN_SyncIo_Disconnect(sio);
    GWEN_SyncIo_free(sio);
    return 0;
  }
  GWEN_Buffer_free(lbuffer);
  GWEN_SyncIo_Disconnect(sio);
  GWEN_SyncIo_free(sio);
  return GWEN_ERROR_BAD_DATA;
}



int AB_ImExporterERI2_Export(AB_IMEXPORTER *ie,
                             AB_IMEXPORTER_CONTEXT *ctx,
			     GWEN_SYNCIO *sio,
			     GWEN_DB_NODE *params){
  AB_IMEXPORTER_ERI2 *ieh;

  assert(ie);
  ieh = GWEN_INHERIT_GETDATA(AB_IMEXPORTER, AB_IMEXPORTER_ERI2, ie);
  assert(ieh);

  return GWEN_ERROR_GENERIC;
}





