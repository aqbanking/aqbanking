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

#include "csv_p.h"
#include <gwenhywfar/debug.h>
#include <gwenhywfar/waitcallback.h>


GWEN_INHERIT(AB_IMEXPORTER, AH_IMEXPORTER_CSV);


AB_IMEXPORTER *csv_factory(AB_BANKING *ab, GWEN_DB_NODE *db){
  AB_IMEXPORTER *ie;
  AH_IMEXPORTER_CSV *ieh;

  ie=AB_ImExporter_new(ab, "csv");
  GWEN_NEW_OBJECT(AH_IMEXPORTER_CSV, ieh);
  GWEN_INHERIT_SETDATA(AB_IMEXPORTER, AH_IMEXPORTER_CSV, ie, ieh,
                       AH_ImExporterCSV_FreeData);
  ieh->dbData=db;
  ieh->dbio=GWEN_DBIO_GetPlugin("csv");
  if (!ieh->dbio) {
    DBG_ERROR(AQBANKING_LOGDOMAIN,
              "GWEN DBIO plugin \"CSV\" not available");
    AB_ImExporter_free(ie);
    return 0;
  }

  AB_ImExporter_SetImportFn(ie, AH_ImExporterCSV_Import);
  AB_ImExporter_SetExportFn(ie, AH_ImExporterCSV_Export);
  AB_ImExporter_SetCheckFileFn(ie, AH_ImExporterCSV_CheckFile);
  return ie;
}



void AH_ImExporterCSV_FreeData(void *bp, void *p){
  AH_IMEXPORTER_CSV *ieh;

  ieh=(AH_IMEXPORTER_CSV*)p;
  GWEN_FREE_OBJECT(ieh);
}



int AH_ImExporterCSV_Import(AB_IMEXPORTER *ie,
                            AB_IMEXPORTER_CONTEXT *ctx,
                            GWEN_BUFFEREDIO *bio,
                            GWEN_DB_NODE *params){
  AH_IMEXPORTER_CSV *ieh;
  GWEN_DB_NODE *dbData;
  GWEN_DB_NODE *dbSubParams;
  int rv;

  assert(ie);
  ieh=GWEN_INHERIT_GETDATA(AB_IMEXPORTER, AH_IMEXPORTER_CSV, ie);
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
    DBG_ERROR(AQBANKING_LOGDOMAIN, "Error importing data (%d)", rv);
    GWEN_WaitCallback_Log(GWEN_LoggerLevelError,
                          "Error importing data");
    GWEN_DB_Group_free(dbData);
    return AB_ERROR_GENERIC;
  }

  /* transform DB to transactions */
  GWEN_WaitCallback_Log(GWEN_LoggerLevelNotice,
                        "Data imported, transforming to transactions");
  rv=AH_ImExporterCSV__ImportFromGroup(ctx, dbData, params);
  if (rv) {
    GWEN_WaitCallback_Log(GWEN_LoggerLevelError,
                          "Error importing data");
    GWEN_DB_Group_free(dbData);
    return rv;
  }

  GWEN_DB_Group_free(dbData);
  return 0;
}



int AH_ImExporterCSV__ImportFromGroup(AB_IMEXPORTER_CONTEXT *ctx,
                                      GWEN_DB_NODE *db,
                                      GWEN_DB_NODE *dbParams) {
  GWEN_DB_NODE *dbT;
  const char *dateFormat;
  int inUtc;

  dbT=GWEN_DB_GetFirstGroup(db);

  dateFormat=GWEN_DB_GetCharValue(dbParams, "dateFormat", 0, "YYYY/MM/DD");
  inUtc=GWEN_DB_GetIntValue(dbParams, "utc", 0, 0);

  while(dbT) {
    int matches;
    int i;
    const char *p;
    const char *gn;

    /* check whether the name of the current groups matches */
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
    } /* for */

    if (!matches && i==0) {
      /* no names given, check default */
      if ((strcasecmp(GWEN_DB_GroupName(dbT), "transaction")==0) ||
          (strcasecmp(GWEN_DB_GroupName(dbT), "debitnote")==0))
        matches=1;
    }

    if (matches) {
      AB_TRANSACTION *t;
      const char *p;

      t=AB_Transaction_fromDb(dbT);
      if (!t) {
        DBG_ERROR(AQBANKING_LOGDOMAIN, "Error in config file");
        GWEN_WaitCallback_Log(GWEN_LoggerLevelError,
                              "Error in config file");
        return AB_ERROR_GENERIC;
      }

      /* translate date */
      p=GWEN_DB_GetCharValue(dbT, "date", 0, 0);
      if (p) {
	GWEN_TIME *ti;

        ti=AB_ImExporter_DateFromString(p, dateFormat, inUtc);
        if (ti)
          AB_Transaction_SetDate(t, ti);
        GWEN_Time_free(ti);
      }

      /* translate valutaDate */
      p=GWEN_DB_GetCharValue(dbT, "valutaDate", 0, 0);
      if (p) {
	GWEN_TIME *ti;

        ti=AB_ImExporter_DateFromString(p, dateFormat, inUtc);
        if (ti)
          AB_Transaction_SetValutaDate(t, ti);
        GWEN_Time_free(ti);
      }

      DBG_NOTICE(AQBANKING_LOGDOMAIN, "Adding transaction");
      AB_ImExporterContext_AddTransaction(ctx, t);
    }
    else {
      int rv;

      /* not a transaction, check subgroups */
      rv=AH_ImExporterCSV__ImportFromGroup(ctx, dbT, dbParams);
      if (rv)
        return rv;
    }
    dbT=GWEN_DB_GetNextGroup(dbT);
  } // while

  return 0;
}



int AH_ImExporterCSV_CheckFile(AB_IMEXPORTER *ie, const char *fname){
  AH_IMEXPORTER_CSV *ieh;
  GWEN_DBIO_CHECKFILE_RESULT rv;

  assert(ie);
  ieh=GWEN_INHERIT_GETDATA(AB_IMEXPORTER, AH_IMEXPORTER_CSV, ie);
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



int AH_ImExporterCSV_Export(AB_IMEXPORTER *ie,
                            AB_IMEXPORTER_CONTEXT *ctx,
                            GWEN_BUFFEREDIO *bio,
                            GWEN_DB_NODE *params){
  AH_IMEXPORTER_CSV *ieh;
  AB_IMEXPORTER_ACCOUNTINFO *ai;
  GWEN_DB_NODE *dbData;
  GWEN_DB_NODE *dbSubParams;
  int rv;
  const char *dateFormat;
  int inUtc;

  assert(ie);
  ieh=GWEN_INHERIT_GETDATA(AB_IMEXPORTER, AH_IMEXPORTER_CSV, ie);
  assert(ieh);
  assert(ieh->dbio);

  dbSubParams=GWEN_DB_GetGroup(params, GWEN_PATH_FLAGS_NAMEMUSTEXIST,
                               "params");
  dateFormat=GWEN_DB_GetCharValue(params, "dateFormat", 0,
				  "YYYY/MM/DD");
  inUtc=GWEN_DB_GetIntValue(params, "utc", 0, 0);

  /* create db, store transactions in it */
  dbData=GWEN_DB_Group_new("transactions");
  ai=AB_ImExporterContext_GetFirstAccountInfo(ctx);
  while(ai) {
    const AB_TRANSACTION *t;

    t=AB_ImExporterAccountInfo_GetFirstTransaction(ai);
    while(t) {
      GWEN_DB_NODE *dbTransaction;
      const GWEN_TIME *ti;
      AB_SPLIT *sp;

      dbTransaction=GWEN_DB_Group_new("transaction");
      rv=AB_Transaction_toDb(t, dbTransaction);
      if (rv) {
        DBG_ERROR(AQBANKING_LOGDOMAIN,
                  "Could not transform transaction to db");
        GWEN_WaitCallback_Log(GWEN_LoggerLevelError,
                              "Error transforming data to db");
        GWEN_DB_Group_free(dbData);
        GWEN_DB_Group_free(dbTransaction);
        return AB_ERROR_GENERIC;
      }

      /* if transaction contains splits:
       * Take the first split and store it within the transaction thus
       * overwriting possibly existing data in the transaction itself.
       * However, if there actually ARE splits then the corresponding data
       * within the transaction is NULL anyway so this step doesn't hurt. */
      sp=AB_Split_List_First(AB_Transaction_GetSplits(t));
      if (sp) {
        rv=AB_Split_toDb(sp, dbTransaction);
        if (rv) {
          DBG_ERROR(AQBANKING_LOGDOMAIN,
                    "Could not transform split to db");
          GWEN_WaitCallback_Log(GWEN_LoggerLevelError,
                                "Error transforming data to db");
          GWEN_DB_Group_free(dbData);
          GWEN_DB_Group_free(dbTransaction);
          return AB_ERROR_GENERIC;
        }
      }

      /* transform dates */
      GWEN_DB_DeleteGroup(dbTransaction, "date");
      GWEN_DB_DeleteGroup(dbTransaction, "valutaDate");

      ti=AB_Transaction_GetDate(t);
      if (ti) {
	GWEN_BUFFER *tbuf;
        int rv;

	tbuf=GWEN_Buffer_new(0, 32, 0, 1);
	if (inUtc)
	  rv=GWEN_Time_toUtcString(ti, dateFormat, tbuf);
	else
	  rv=GWEN_Time_toString(ti, dateFormat, tbuf);
	if (rv) {
	  DBG_WARN(AQBANKING_LOGDOMAIN, "Bad date format string/date");
	}
	else
	  GWEN_DB_SetCharValue(dbTransaction, GWEN_DB_FLAGS_OVERWRITE_VARS,
			       "date", GWEN_Buffer_GetStart(tbuf));
        GWEN_Buffer_free(tbuf);
      }

      ti=AB_Transaction_GetValutaDate(t);
      if (ti) {
	GWEN_BUFFER *tbuf;

	tbuf=GWEN_Buffer_new(0, 32, 0, 1);
	if (inUtc)
	  rv=GWEN_Time_toUtcString(ti, dateFormat, tbuf);
	else
	  rv=GWEN_Time_toString(ti, dateFormat, tbuf);
	if (rv) {
	  DBG_WARN(AQBANKING_LOGDOMAIN, "Bad date format string/date");
	}
	else
	  GWEN_DB_SetCharValue(dbTransaction, GWEN_DB_FLAGS_OVERWRITE_VARS,
			       "valutaDate", GWEN_Buffer_GetStart(tbuf));
	GWEN_Buffer_free(tbuf);
      }

      /* add transaction db */
      GWEN_DB_AddGroup(dbData, dbTransaction);
      t=AB_ImExporterAccountInfo_GetNextTransaction(ai);
    }
    ai=AB_ImExporterContext_GetNextAccountInfo(ctx);
  }


  rv=GWEN_DBIO_Export(ieh->dbio,
                      bio,
                      GWEN_DB_FLAGS_DEFAULT,
                      dbData,
                      dbSubParams);
  if (rv) {
    DBG_ERROR(AQBANKING_LOGDOMAIN, "Error exporting data (%d)", rv);
    GWEN_WaitCallback_Log(GWEN_LoggerLevelError,
                          "Error exporting data");
    GWEN_DB_Group_free(dbData);
    return AB_ERROR_GENERIC;
  }
  GWEN_DB_Group_free(dbData);

  return 0;
}




