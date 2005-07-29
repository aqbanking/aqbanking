/***************************************************************************
 $RCSfile$
                             -------------------
    cvs         : $Id$
    begin       : Mon Mar 01 2004
    copyright   : (C) 2004 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "swift_p.h"
#include "i18n_l.h"
#include <aqbanking/banking.h>
#include <gwenhywfar/debug.h>
#include <gwenhywfar/misc.h>
#include <gwenhywfar/waitcallback.h>
#include <gwenhywfar/inherit.h>


GWEN_INHERIT(AB_IMEXPORTER, AH_IMEXPORTER_SWIFT);


AB_IMEXPORTER *swift_factory(AB_BANKING *ab, GWEN_DB_NODE *db){
  AB_IMEXPORTER *ie;
  AH_IMEXPORTER_SWIFT *ieh;

  ie=AB_ImExporter_new(ab, "swift");
  GWEN_NEW_OBJECT(AH_IMEXPORTER_SWIFT, ieh);
  GWEN_INHERIT_SETDATA(AB_IMEXPORTER, AH_IMEXPORTER_SWIFT, ie, ieh,
                       AH_ImExporterSWIFT_FreeData);
  ieh->dbData=db;
  ieh->dbio=GWEN_DBIO_GetPlugin("swift");
  if (!ieh->dbio) {
    DBG_ERROR(AQBANKING_LOGDOMAIN,
              "GWEN DBIO plugin \"SWIFT\" not available");
    AB_ImExporter_free(ie);
    return 0;
  }

  AB_ImExporter_SetImportFn(ie, AH_ImExporterSWIFT_Import);
  AB_ImExporter_SetCheckFileFn(ie, AH_ImExporterSWIFT_CheckFile);
  return ie;
}



void AH_ImExporterSWIFT_FreeData(void *bp, void *p){
  AH_IMEXPORTER_SWIFT *ieh;

  ieh=(AH_IMEXPORTER_SWIFT*)p;
  GWEN_FREE_OBJECT(ieh);
}



int AH_ImExporterSWIFT_Import(AB_IMEXPORTER *ie,
                             AB_IMEXPORTER_CONTEXT *ctx,
                             GWEN_BUFFEREDIO *bio,
                             GWEN_DB_NODE *params){
  AH_IMEXPORTER_SWIFT *ieh;
  GWEN_DB_NODE *dbData;
  GWEN_DB_NODE *dbSubParams;
  int rv;

  assert(ie);
  ieh=GWEN_INHERIT_GETDATA(AB_IMEXPORTER, AH_IMEXPORTER_SWIFT, ie);
  assert(ieh);
  assert(ieh->dbio);

  dbSubParams=GWEN_DB_GetGroup(params, GWEN_PATH_FLAGS_NAMEMUSTEXIST,
			       "params");
  dbData=GWEN_DB_Group_new("transactions");
  rv=GWEN_DBIO_Import(ieh->dbio,
                      bio,
                      GWEN_DB_FLAGS_DEFAULT |
                      GWEN_PATH_FLAGS_CREATE_GROUP,
                      dbData,
		      dbSubParams);
  if (rv) {
    DBG_ERROR(AQBANKING_LOGDOMAIN, "Error importing data");
    GWEN_WaitCallback_Log(GWEN_LoggerLevelError,
                          "Error importing data");
    GWEN_DB_Group_free(dbData);
    return rv;
  }

  /* transform DB to transactions */
  GWEN_WaitCallback_Log(GWEN_LoggerLevelError,
                        "Data imported, transforming to transactions");
  rv=AH_ImExporterSWIFT__ImportFromGroup(ctx, dbData, params);
  if (rv) {
    GWEN_WaitCallback_Log(GWEN_LoggerLevelError,
                          "Error importing data");
    GWEN_DB_Group_free(dbData);
    return rv;
  }

  GWEN_DB_Group_free(dbData);
  return 0;
}



int AH_ImExporterSWIFT__ImportFromGroup(AB_IMEXPORTER_CONTEXT *ctx,
                                        GWEN_DB_NODE *db,
                                        GWEN_DB_NODE *dbParams) {
  GWEN_DB_NODE *dbT;
  GWEN_TYPE_UINT64 cnt=0;
  GWEN_TYPE_UINT64 done;

  /* first count the groups */
  dbT=GWEN_DB_GetFirstGroup(db);
  while(dbT) {
    cnt++;
    dbT=GWEN_DB_GetNextGroup(dbT);
  } /* while */

  /* enter waitcallback context */
  DBG_ERROR(AQBANKING_LOGDOMAIN, "Entering callback...");

  GWEN_WaitCallback_EnterWithText(GWEN_WAITCALLBACK_ID_SIMPLE_PROGRESS,
                                  I18N("Importing transactions ..."),
                                  I18N("transaction(s)"),
                                  GWEN_WAITCALLBACK_FLAGS_NO_REUSE);
  GWEN_WaitCallback_SetProgressTotal(cnt);
  GWEN_WaitCallback_SetProgressPos(0);

  done=0;
  dbT=GWEN_DB_GetFirstGroup(db);
  while(dbT) {
    int matches;
    int i;
    const char *p;
    const char *gn;

    // check whether the name of the current groups matches
    matches=0;
    gn=GWEN_DB_GroupName(dbT);
    for (i=0; ; i++) {
      p=GWEN_DB_GetCharValue(dbParams, "groupNames", i, 0);
      if (!p)
        break;
      if (strcasecmp(gn, p)==0) {
        matches=1;
        break;
      }
    } // for

    if (!matches && i==0) {
      // no names given, check default
      if ((strcasecmp(GWEN_DB_GroupName(dbT), "transaction")==0) ||
          (strcasecmp(GWEN_DB_GroupName(dbT), "debitnote")==0))
        matches=1;
    }

    if (matches) {
      AB_TRANSACTION *t;

      t=AB_Transaction_fromDb(dbT);
      if (!t) {
        DBG_ERROR(AQBANKING_LOGDOMAIN, "Error in config file");
        GWEN_WaitCallback_Log(GWEN_LoggerLevelError,
                              "Error in config file");
        GWEN_WaitCallback_Leave();
        return AB_ERROR_GENERIC;
      }
      DBG_DEBUG(AQBANKING_LOGDOMAIN, "Adding transaction");
      AB_ImExporterContext_AddTransaction(ctx, t);
    }
    else {
      int rv;

      // not a transaction, check subgroups
      rv=AH_ImExporterSWIFT__ImportFromGroup(ctx, dbT, dbParams);
      if (rv) {
        GWEN_WaitCallback_Leave();
        return rv;
      }
    }
    done++;
    if (GWEN_WaitCallbackProgress(done)==GWEN_WaitCallbackResult_Abort) {
      GWEN_WaitCallback_Leave();
      return AB_ERROR_USER_ABORT;
    }
    GWEN_WaitCallback_SetProgressPos(done);
    dbT=GWEN_DB_GetNextGroup(dbT);
  } // while

  return 0;
}



int AH_ImExporterSWIFT_CheckFile(AB_IMEXPORTER *ie, const char *fname){
  AH_IMEXPORTER_SWIFT *ieh;
  GWEN_DBIO_CHECKFILE_RESULT rv;

  assert(ie);
  ieh=GWEN_INHERIT_GETDATA(AB_IMEXPORTER, AH_IMEXPORTER_SWIFT, ie);
  assert(ieh);
  assert(ieh->dbio);

  rv=GWEN_DBIO_CheckFile(ieh->dbio, fname);
  switch(rv) {
  case GWEN_DBIO_CheckFileResultOk:      return 0;
  case GWEN_DBIO_CheckFileResultNotOk:   return AB_ERROR_BAD_DATA;
  case GWEN_DBIO_CheckFileResultUnknown: return AB_ERROR_UNKNOWN;
  default:                               return AB_ERROR_GENERIC;
  } /* switch */
}







