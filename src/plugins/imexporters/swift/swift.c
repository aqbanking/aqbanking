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

#include "swift_p.h"
#include "i18n_l.h"

#include <aqbanking/banking.h>
#include <aqbanking/accstatus.h>

#include <gwenhywfar/debug.h>
#include <gwenhywfar/misc.h>
#include <gwenhywfar/gui.h>
#include <gwenhywfar/inherit.h>



GWEN_INHERIT(AB_IMEXPORTER, AH_IMEXPORTER_SWIFT);



GWEN_PLUGIN *imexporter_swift_factory(GWEN_PLUGIN_MANAGER *pm,
				      const char *name,
				      const char *fileName) {
  GWEN_PLUGIN *pl;

  pl=AB_Plugin_ImExporter_new(pm, name, fileName);
  assert(pl);

  AB_Plugin_ImExporter_SetFactoryFn(pl, AB_Plugin_ImExporterSWIFT_Factory);

  return pl;
}



AB_IMEXPORTER *AB_Plugin_ImExporterSWIFT_Factory(GWEN_PLUGIN *pl,
						 AB_BANKING *ab){
  AB_IMEXPORTER *ie;
  AH_IMEXPORTER_SWIFT *ieh;

  ie=AB_ImExporter_new(ab, "swift");
  GWEN_NEW_OBJECT(AH_IMEXPORTER_SWIFT, ieh);
  GWEN_INHERIT_SETDATA(AB_IMEXPORTER, AH_IMEXPORTER_SWIFT, ie, ieh,
                       AH_ImExporterSWIFT_FreeData);
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



void GWENHYWFAR_CB AH_ImExporterSWIFT_FreeData(void *bp, void *p){
  AH_IMEXPORTER_SWIFT *ieh;

  ieh=(AH_IMEXPORTER_SWIFT*)p;
  GWEN_DBIO_free(ieh->dbio);
  GWEN_FREE_OBJECT(ieh);
}



int AH_ImExporterSWIFT_Import(AB_IMEXPORTER *ie,
                              AB_IMEXPORTER_CONTEXT *ctx,
                              GWEN_SYNCIO *sio,
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
  GWEN_Gui_ProgressLog(0, GWEN_LoggerLevel_Debug,
		       I18N("Reading file..."));

  rv=GWEN_DBIO_Import(ieh->dbio,
		      sio,
		      dbData,
		      dbSubParams,
		      GWEN_DB_FLAGS_DEFAULT |
		      GWEN_PATH_FLAGS_CREATE_GROUP);
  if (rv) {
    DBG_ERROR(AQBANKING_LOGDOMAIN, "Error importing data (%d)", rv);
    GWEN_Gui_ProgressLog(0, GWEN_LoggerLevel_Error,
			 I18N("Error importing data"));
    GWEN_DB_Group_free(dbData);
    return GWEN_ERROR_BAD_DATA;
  }

  /* transform DB to transactions */
  GWEN_Gui_ProgressLog(0, GWEN_LoggerLevel_Debug,
		       "Data imported, transforming to transactions");
  rv=AH_ImExporterSWIFT__ImportFromGroup(ctx, dbData, params);
  if (rv) {
    GWEN_Gui_ProgressLog(0, GWEN_LoggerLevel_Error,
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
  uint32_t progressId;

  progressId=GWEN_Gui_ProgressStart(GWEN_GUI_PROGRESS_DELAY |
				    GWEN_GUI_PROGRESS_ALLOW_EMBED |
				    GWEN_GUI_PROGRESS_SHOW_PROGRESS |
				    GWEN_GUI_PROGRESS_SHOW_ABORT,
				    I18N("Importing parsed data..."),
				    NULL,
				    GWEN_DB_Groups_Count(db),
				    0);
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
	GWEN_Gui_ProgressLog(0, GWEN_LoggerLevel_Error,
			     I18N("Error in config file"));
	return GWEN_ERROR_GENERIC;
      }
      DBG_DEBUG(AQBANKING_LOGDOMAIN, "Adding transaction");
      GWEN_Gui_ProgressLog(0, GWEN_LoggerLevel_Debug,
			   I18N("Adding transaction"));
      AB_ImExporterContext_AddTransaction(ctx, t);
    }
    else if (strcasecmp(GWEN_DB_GroupName(dbT), "startSaldo")==0) {
      /* ignore start saldo, but since the existence of this group shows
       * that we in fact are within a swift DB group we don't need recursions.
       */
    }
    else if (strcasecmp(GWEN_DB_GroupName(dbT), "endSaldo")==0) {
      GWEN_DB_NODE *dbX;
      GWEN_TIME *ti=0;
      const char *bankCode;
      const char *accountNumber;

      bankCode=GWEN_DB_GetCharValue(dbT, "localBankCode", 0, 0);
      accountNumber=GWEN_DB_GetCharValue(dbT, "localAccountNumber", 0, 0);

      dbX=GWEN_DB_GetGroup(dbT, GWEN_PATH_FLAGS_NAMEMUSTEXIST, "date");
      if (dbX)
        ti=GWEN_Time_fromDb(dbX);
      dbX=GWEN_DB_GetGroup(dbT, GWEN_PATH_FLAGS_NAMEMUSTEXIST, "value");
      if (dbX) {
        AB_VALUE *v;

        v=AB_Value_fromDb(dbX);
        if (v) {
          AB_BALANCE *bal;
          AB_IMEXPORTER_ACCOUNTINFO *iea;
          AB_ACCOUNT_STATUS *as;

          bal=AB_Balance_new(v, ti);
          AB_Value_free(v);
          as=AB_AccountStatus_new();
          if (ti)
            AB_AccountStatus_SetTime(as, ti);
          AB_AccountStatus_SetNotedBalance(as, bal);
          AB_Balance_free(bal);
          iea=AB_ImExporterContext_GetAccountInfo(ctx,
                                                  bankCode,
                                                  accountNumber);
          AB_ImExporterAccountInfo_AddAccountStatus(iea, as);
        }
      }
      GWEN_Time_free(ti);
    }
    else {
      int rv;

      // not a transaction, check subgroups
      rv=AH_ImExporterSWIFT__ImportFromGroup(ctx, dbT, dbParams);
      if (rv) {
	GWEN_Gui_ProgressEnd(progressId);
	return rv;
      }
    }

    if (GWEN_Gui_ProgressAdvance(progressId, GWEN_GUI_PROGRESS_ONE)==
	GWEN_ERROR_USER_ABORTED) {
      GWEN_Gui_ProgressLog(0, GWEN_LoggerLevel_Error,
			   I18N("Aborted by user"));
      GWEN_Gui_ProgressEnd(progressId);
      return GWEN_ERROR_USER_ABORTED;
    }
    dbT=GWEN_DB_GetNextGroup(dbT);
  } // while

  GWEN_Gui_ProgressEnd(progressId);
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
  case GWEN_DBIO_CheckFileResultNotOk:   return GWEN_ERROR_BAD_DATA;
  case GWEN_DBIO_CheckFileResultUnknown: return AB_ERROR_INDIFFERENT;
  default:                               return GWEN_ERROR_GENERIC;
  } /* switch */
}







