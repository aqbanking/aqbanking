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

#include "openhbci1_p.h"
#include "i18n_l.h"
#include <aqbanking/banking.h>
#include <gwenhywfar/debug.h>
#include <gwenhywfar/misc.h>
#include <gwenhywfar/gui.h>
#include <gwenhywfar/inherit.h>


GWEN_INHERIT(AB_IMEXPORTER, AH_IMEXPORTER_OPENHBCI1);



GWEN_PLUGIN *imexporter_openhbci1_factory(GWEN_PLUGIN_MANAGER *pm,
					  const char *name,
					  const char *fileName) {
  GWEN_PLUGIN *pl;

  pl=AB_Plugin_ImExporter_new(pm, name, fileName);
  assert(pl);

  AB_Plugin_ImExporter_SetFactoryFn(pl, AB_Plugin_ImExporterOpenHBCI1_Factory);

  return pl;
}



AB_IMEXPORTER *AB_Plugin_ImExporterOpenHBCI1_Factory(GWEN_PLUGIN *pl,
						     AB_BANKING *ab){
  AB_IMEXPORTER *ie;
  AH_IMEXPORTER_OPENHBCI1 *ieh;

  ie=AB_ImExporter_new(ab, "openhbci1");
  GWEN_NEW_OBJECT(AH_IMEXPORTER_OPENHBCI1, ieh);
  GWEN_INHERIT_SETDATA(AB_IMEXPORTER, AH_IMEXPORTER_OPENHBCI1, ie, ieh,
                       AH_ImExporterOpenHBCI1_FreeData);
  ieh->dbio=GWEN_DBIO_GetPlugin("olddb");
  if (!ieh->dbio) {
    DBG_ERROR(AQBANKING_LOGDOMAIN,
              "GWEN DBIO plugin \"OldDb\" not available");
    AB_ImExporter_free(ie);
    return 0;
  }

  AB_ImExporter_SetImportFn(ie, AH_ImExporterOpenHBCI1_Import);
  AB_ImExporter_SetExportFn(ie, AH_ImExporterOpenHBCI1_Export);
  AB_ImExporter_SetCheckFileFn(ie, AH_ImExporterOpenHBCI1_CheckFile);
  return ie;
}



void GWENHYWFAR_CB AH_ImExporterOpenHBCI1_FreeData(void *bp, void *p){
  AH_IMEXPORTER_OPENHBCI1 *ieh;

  ieh=(AH_IMEXPORTER_OPENHBCI1*)p;
  GWEN_DBIO_free(ieh->dbio);
  GWEN_FREE_OBJECT(ieh);
}



int AH_ImExporterOpenHBCI1_Import(AB_IMEXPORTER *ie,
				  AB_IMEXPORTER_CONTEXT *ctx,
				  GWEN_SYNCIO *sio,
				  GWEN_DB_NODE *params){
  AH_IMEXPORTER_OPENHBCI1 *ieh;
  GWEN_DB_NODE *dbData;
  GWEN_DB_NODE *dbSubParams;
  int rv;

  assert(ie);
  ieh=GWEN_INHERIT_GETDATA(AB_IMEXPORTER, AH_IMEXPORTER_OPENHBCI1, ie);
  assert(ieh);
  assert(ieh->dbio);

  dbSubParams=GWEN_DB_GetGroup(params, GWEN_PATH_FLAGS_NAMEMUSTEXIST,
			       "params");
  dbData=GWEN_DB_Group_new("transactions");
  rv=GWEN_DBIO_Import(ieh->dbio,
		      sio,
                      dbData,
		      dbSubParams,
		      GWEN_DB_FLAGS_DEFAULT |
		      GWEN_PATH_FLAGS_CREATE_GROUP);
  if (rv) {
    DBG_ERROR(AQBANKING_LOGDOMAIN, "Error importing data (%d)", rv);
    GWEN_Gui_ProgressLog(0, GWEN_LoggerLevel_Error,
			 "Error importing data");
    GWEN_DB_Group_free(dbData);
    return GWEN_ERROR_GENERIC;
  }

  /* transform DB to transactions */
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
  rv=AH_ImExporterOpenHBCI1__ImportFromGroup(ctx, dbData, params);
  if (rv) {
    GWEN_Gui_ProgressLog(0, GWEN_LoggerLevel_Error,
			 "Error importing data");
    GWEN_DB_Group_free(dbData);
    return rv;
  }

  GWEN_DB_Group_free(dbData);
  return 0;
}



int AH_ImExporterOpenHBCI1__ImportFromGroup(AB_IMEXPORTER_CONTEXT *ctx,
					    GWEN_DB_NODE *db,
					    GWEN_DB_NODE *dbParams) {
  GWEN_DB_NODE *dbBanks;
  const char *dateFormat;
  int inUtc;

  dateFormat=GWEN_DB_GetCharValue(dbParams, "dateFormat", 0, "YYYYMMDD");
  inUtc=GWEN_DB_GetIntValue(dbParams, "utc", 0, 0);

  dbBanks=GWEN_DB_GetGroup(db, GWEN_PATH_FLAGS_NAMEMUSTEXIST, "bank");
  if (dbBanks) {
    GWEN_DB_NODE *dbBank;

    dbBank=GWEN_DB_GetFirstGroup(dbBanks);
    if (!dbBank) {
      DBG_ERROR(AQBANKING_LOGDOMAIN, "No bank groups");
    }
    while(dbBank) {
      GWEN_DB_NODE *dbAccounts;

      dbAccounts=GWEN_DB_GetGroup(dbBank, GWEN_PATH_FLAGS_NAMEMUSTEXIST,
				  "account");
      if (dbAccounts) {
	GWEN_DB_NODE *dbAccount;

	dbAccount=GWEN_DB_GetFirstGroup(dbAccounts);
	if (!dbAccount) {
	  DBG_ERROR(AQBANKING_LOGDOMAIN, "No account groups");
	}
	while(dbAccount) {
	  GWEN_DB_NODE *dbT;
	  const char *bankId;
	  const char *accountId;

	  bankId=GWEN_DB_GetCharValue(dbAccount, "params/institute", 0, 0);
	  accountId=GWEN_DB_GetCharValue(dbAccount,
					 "params/accountNumber", 0, 0);
	  dbT=GWEN_DB_FindFirstGroup(dbAccount, "transaction");
	  if (!dbT) {
	    DBG_ERROR(AQBANKING_LOGDOMAIN, "No transaction groups");
	  }
	  while(dbT) {
	    AB_TRANSACTION *t;
	    const char *p;
	    int i;

            t=AB_Transaction_new();
	    AB_Transaction_SetLocalBankCode(t, bankId);
	    AB_Transaction_SetLocalAccountNumber(t, accountId);
	    p=GWEN_DB_GetCharValue(dbT, "otherInstitute", 0, 0);
	    AB_Transaction_SetRemoteBankCode(t, p);
	    p=GWEN_DB_GetCharValue(dbT, "otherId", 0, 0);
	    AB_Transaction_SetRemoteAccountNumber(t, p);
	    p=GWEN_DB_GetCharValue(dbT, "primanota", 0, 0);
	    AB_Transaction_SetPrimanota(t, p);
	    p=GWEN_DB_GetCharValue(dbT, "key", 0, 0);
	    AB_Transaction_SetTransactionKey(t, p);
	    p=GWEN_DB_GetCharValue(dbT, "text", 0, 0);
	    AB_Transaction_SetTransactionText(t, p);
	    AB_Transaction_SetTextKey(t, GWEN_DB_GetIntValue(dbT,
							     "code",
							     0, 53));
	    for (i=0; ; i++) {
	      p=GWEN_DB_GetCharValue(dbT, "description", i, 0);
	      if (!p)
		break;
	      AB_Transaction_AddPurpose(t, p, 0);
	    }

	    for (i=0; ; i++) {
	      p=GWEN_DB_GetCharValue(dbT, "otherName", i, 0);
	      if (!p)
		break;
	      AB_Transaction_AddRemoteName(t, p, 0);
	    }

	    p=GWEN_DB_GetCharValue(dbT, "customerReference", 0, 0);
	    AB_Transaction_SetCustomerReference(t, p);
	    p=GWEN_DB_GetCharValue(dbT, "bankReference", 0, 0);
	    AB_Transaction_SetBankReference(t, p);

	    p=GWEN_DB_GetCharValue(dbT, "value", 0, 0);
	    if (p) {
	      AB_VALUE *val;

	      val=AB_Value_fromString(p);
	      if (!val) {
		DBG_ERROR(AQBANKING_LOGDOMAIN, "Bad value");
	      }
	      else {
		if (AB_Value_GetCurrency(val)==0)
		  AB_Value_SetCurrency(val, "DEM");
                AB_Transaction_SetValue(t, val);
	      }
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

	    dbT=GWEN_DB_FindNextGroup(dbT, "transaction");
	  } /* while dbT */

	  dbAccount=GWEN_DB_GetNextGroup(dbAccount);
	} /* while dbAccount */

      } /* if dbAccounts */
      else {
	DBG_ERROR(AQBANKING_LOGDOMAIN, "No account group");
      }
      dbBank=GWEN_DB_GetNextGroup(dbBank);
    } /* while dbBank */
  } /* if dbBanks */
  else {
    DBG_ERROR(AQBANKING_LOGDOMAIN, "No bank group");
  }

  return 0;
}



int AH_ImExporterOpenHBCI1_CheckFile(AB_IMEXPORTER *ie, const char *fname){
  AH_IMEXPORTER_OPENHBCI1 *ieh;
  GWEN_DBIO_CHECKFILE_RESULT rv;

  assert(ie);
  ieh=GWEN_INHERIT_GETDATA(AB_IMEXPORTER, AH_IMEXPORTER_OPENHBCI1, ie);
  assert(ieh);
  assert(ieh->dbio);

  rv=GWEN_DBIO_CheckFile(ieh->dbio, fname);
  switch(rv) {
  case GWEN_DBIO_CheckFileResultOk:      return AB_ERROR_INDIFFERENT;
  case GWEN_DBIO_CheckFileResultNotOk:   return GWEN_ERROR_BAD_DATA;
  case GWEN_DBIO_CheckFileResultUnknown: return AB_ERROR_INDIFFERENT;
  default:                               return GWEN_ERROR_GENERIC;
  } /* switch */
}



int AH_ImExporterOpenHBCI1_Export(AB_IMEXPORTER *ie,
				  AB_IMEXPORTER_CONTEXT *ctx,
                                  GWEN_SYNCIO *sio,
				  GWEN_DB_NODE *params){
  AH_IMEXPORTER_OPENHBCI1 *ieh;
  AB_IMEXPORTER_ACCOUNTINFO *ai;
  GWEN_DB_NODE *dbData;
  GWEN_DB_NODE *dbSubParams;
  int rv;
  const char *dateFormat;
  int inUtc;

  assert(ie);
  ieh=GWEN_INHERIT_GETDATA(AB_IMEXPORTER, AH_IMEXPORTER_OPENHBCI1, ie);
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

      dbTransaction=GWEN_DB_Group_new("transaction");
      rv=AB_Transaction_toDb(t, dbTransaction);
      if (rv) {
        DBG_ERROR(AQBANKING_LOGDOMAIN,
                  "Could not transform transaction to db");
	GWEN_Gui_ProgressLog(0, GWEN_LoggerLevel_Error,
			     "Error transforming data to db");
        GWEN_DB_Group_free(dbData);
        GWEN_DB_Group_free(dbTransaction);
        return GWEN_ERROR_GENERIC;
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
                      sio,
                      dbData,
		      dbSubParams,
		      GWEN_DB_FLAGS_DEFAULT);
  if (rv) {
    DBG_ERROR(AQBANKING_LOGDOMAIN, "Error exporting data (%d)", rv);
    GWEN_Gui_ProgressLog(0, GWEN_LoggerLevel_Error,
			 "Error exporting data");
    GWEN_DB_Group_free(dbData);
    return GWEN_ERROR_GENERIC;
  }
  GWEN_DB_Group_free(dbData);

  return 0;
}




