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

#include "dtaus_p.h"
#include "i18n_l.h"

#include <aqbanking/banking.h>
#include <gwenhywfar/debug.h>
#include <gwenhywfar/misc.h>
#include <gwenhywfar/gui.h>
#include <gwenhywfar/inherit.h>


GWEN_INHERIT(AB_IMEXPORTER, AH_IMEXPORTER_DTAUS);



GWEN_PLUGIN *imexporter_dtaus_factory(GWEN_PLUGIN_MANAGER *pm,
				      const char *name,
				      const char *fileName) {
  GWEN_PLUGIN *pl;

  pl=AB_Plugin_ImExporter_new(pm, name, fileName);
  assert(pl);

  AB_Plugin_ImExporter_SetFactoryFn(pl, AB_Plugin_ImExporterDTAUS_Factory);

  return pl;
}


/** @TODO: Need to prepare transactions befor exporting them... */

AB_IMEXPORTER *AB_Plugin_ImExporterDTAUS_Factory(GWEN_PLUGIN *pl,
						 AB_BANKING *ab){
  AB_IMEXPORTER *ie;
  AH_IMEXPORTER_DTAUS *ieh;

  ie=AB_ImExporter_new(ab, "dtaus");
  GWEN_NEW_OBJECT(AH_IMEXPORTER_DTAUS, ieh);
  GWEN_INHERIT_SETDATA(AB_IMEXPORTER, AH_IMEXPORTER_DTAUS, ie, ieh,
                       AH_ImExporterDTAUS_FreeData);
  ieh->dbio=GWEN_DBIO_GetPlugin("dtaus");
  if (!ieh->dbio) {
    DBG_ERROR(AQBANKING_LOGDOMAIN,
              "GWEN DBIO plugin \"DTAUS\" not available");
    AB_ImExporter_free(ie);
    return 0;
  }

  AB_ImExporter_SetImportFn(ie, AH_ImExporterDTAUS_Import);
  AB_ImExporter_SetExportFn(ie, AH_ImExporterDTAUS_Export);
  AB_ImExporter_SetCheckFileFn(ie, AH_ImExporterDTAUS_CheckFile);
  return ie;
}



void GWENHYWFAR_CB AH_ImExporterDTAUS_FreeData(void *bp, void *p){
  AH_IMEXPORTER_DTAUS *ieh;

  ieh=(AH_IMEXPORTER_DTAUS*)p;
  GWEN_DBIO_free(ieh->dbio);
  GWEN_FREE_OBJECT(ieh);
}



int AH_ImExporterDTAUS_Import(AB_IMEXPORTER *ie,
			      AB_IMEXPORTER_CONTEXT *ctx,
			      GWEN_SYNCIO *sio,
			      GWEN_DB_NODE *params){
  AH_IMEXPORTER_DTAUS *ieh;
  GWEN_DB_NODE *dbData;
  GWEN_DB_NODE *dbSubParams;
  int rv;

  assert(ie);
  ieh=GWEN_INHERIT_GETDATA(AB_IMEXPORTER, AH_IMEXPORTER_DTAUS, ie);
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
  if (rv<0) {
    DBG_ERROR(AQBANKING_LOGDOMAIN, "Error importing data (%d)", rv);
    GWEN_Gui_ProgressLog(0,
			 GWEN_LoggerLevel_Error,
			 "Error importing data");
    GWEN_DB_Group_free(dbData);
    return rv;
  }

  /* transform DB to transactions */
  GWEN_Gui_ProgressLog(0, GWEN_LoggerLevel_Notice,
		       "Data imported, transforming to transactions");
  rv=AH_ImExporterDTAUS__ImportFromGroup(ctx, dbData, params);
  if (rv<0) {
    GWEN_Gui_ProgressLog(0,
			 GWEN_LoggerLevel_Error,
			 "Error importing data");
    GWEN_DB_Group_free(dbData);
    return rv;
  }

  GWEN_DB_Group_free(dbData);
  return 0;
}



int AH_ImExporterDTAUS__ImportFromGroup(AB_IMEXPORTER_CONTEXT *ctx,
                                        GWEN_DB_NODE *db,
                                        GWEN_DB_NODE *dbParams) {
  GWEN_DB_NODE *dbT;

  dbT=GWEN_DB_GetFirstGroup(db);

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

      t=AB_Transaction_fromDb(dbT);
      if (!t) {
        DBG_ERROR(AQBANKING_LOGDOMAIN, "Error in config file");
	GWEN_Gui_ProgressLog(0,
			     GWEN_LoggerLevel_Error,
			     "Error in config file");
	return GWEN_ERROR_GENERIC;
      }
      if (strcasecmp(GWEN_DB_GroupName(dbT), "debitnote")==0)
	AB_Transaction_SetType(t, AB_Transaction_TypeDebitNote);
      else
	AB_Transaction_SetType(t, AB_Transaction_TypeTransfer);

      DBG_DEBUG(AQBANKING_LOGDOMAIN, "Adding transaction");
      AB_ImExporterContext_AddTransaction(ctx, t);
    }
    else {
      int rv;

      /* not a transaction, check subgroups */
      rv=AH_ImExporterDTAUS__ImportFromGroup(ctx, dbT, dbParams);
      if (rv)
        return rv;
    }
    dbT=GWEN_DB_GetNextGroup(dbT);
  } /* while */

  return 0;
}



int AH_ImExporterDTAUS_CheckFile(AB_IMEXPORTER *ie, const char *fname){
  AH_IMEXPORTER_DTAUS *ieh;
  GWEN_DBIO_CHECKFILE_RESULT rv;

  assert(ie);
  ieh=GWEN_INHERIT_GETDATA(AB_IMEXPORTER, AH_IMEXPORTER_DTAUS, ie);
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




int AH_ImExporterDTAUS_Export(AB_IMEXPORTER *ie,
			      AB_IMEXPORTER_CONTEXT *ctx,
			      GWEN_SYNCIO *sio,
			      GWEN_DB_NODE *params){
  AH_IMEXPORTER_DTAUS *ieh;
  GWEN_DB_NODE *dbSubParams;
  int rv;
  const char *groupName;
  AB_IMEXPORTER_ACCOUNTINFO *ai;

  assert(ie);
  ieh=GWEN_INHERIT_GETDATA(AB_IMEXPORTER, AH_IMEXPORTER_DTAUS, ie);
  assert(ieh);
  assert(ieh->dbio);

  dbSubParams=GWEN_DB_GetGroup(params, GWEN_PATH_FLAGS_NAMEMUSTEXIST,
			       "params");

  groupName=GWEN_DB_GetCharValue(params, "groupNames", 0, "transfer");

  ai=AB_ImExporterContext_GetFirstAccountInfo(ctx);
  while(ai) {
    GWEN_DB_NODE *dbTransfers;
    const char *aiBankCode;
    const char *aiAccNum;
    const AB_TRANSACTION *t;
    GWEN_DB_NODE *dbCfg;
    const char *localBankCode=NULL;
    const char *localAccountNumber=NULL;
    const char *localName=NULL;

    aiBankCode=AB_ImExporterAccountInfo_GetBankCode(ai);
    aiAccNum=AB_ImExporterAccountInfo_GetAccountNumber(ai);

    localBankCode=aiBankCode;
    localAccountNumber=aiAccNum;

    /* get values for dbCfg from transactions, check for deviations */
    t=AB_ImExporterAccountInfo_GetFirstTransaction(ai);
    while(t) {
      const char *tlocalBankCode;
      const char *tlocalAccountNumber;
      const char *tlocalName;

      tlocalBankCode=AB_Transaction_GetLocalBankCode(t);
      tlocalAccountNumber=AB_Transaction_GetLocalAccountNumber(t);
      tlocalName=AB_Transaction_GetLocalName(t);

      if (tlocalBankCode && !localBankCode)
	localBankCode=tlocalBankCode;
      if (tlocalAccountNumber && !localAccountNumber)
	localAccountNumber=tlocalAccountNumber;
      if (tlocalName && !localName)
	localName=tlocalName;

      if (tlocalBankCode &&
	  localBankCode &&
	  strcasecmp(tlocalBankCode, localBankCode)!=0) {
	GWEN_Gui_ProgressLog(0,
			     GWEN_LoggerLevel_Error,
			     I18N("Transactions with multiple bank codes found"));
	DBG_ERROR(AQBANKING_LOGDOMAIN,
		  "Transactions with multiple bank codes found");
	return GWEN_ERROR_BAD_DATA;
      }

      if (tlocalAccountNumber &&
	  localAccountNumber &&
	  strcasecmp(tlocalAccountNumber, localAccountNumber)!=0) {
	GWEN_Gui_ProgressLog(0,
			     GWEN_LoggerLevel_Error,
			     I18N("Transactions with multiple account numbers found"));
	DBG_ERROR(AQBANKING_LOGDOMAIN,
		  "Transactions with multiple account numbers found");
	return GWEN_ERROR_BAD_DATA;
      }

      if (tlocalName &&
	  localName &&
	  strcasecmp(tlocalName, localName)!=0) {
	GWEN_Gui_ProgressLog(0,
			     GWEN_LoggerLevel_Error,
			     I18N("Transactions with multiple local names found"));
	DBG_ERROR(AQBANKING_LOGDOMAIN,
		  "Transactions with multiple local names found");
	return GWEN_ERROR_BAD_DATA;
      }

      t=AB_ImExporterAccountInfo_GetNextTransaction(ai);
    }

    if (!localBankCode || !localAccountNumber || !localName) {
      DBG_ERROR(AQBANKING_LOGDOMAIN,
		"Missing local account/name specification");
      return GWEN_ERROR_BAD_DATA;
    }

    /* now create and setup dbCfg */
    if (dbSubParams)
      dbCfg=GWEN_DB_Group_dup(dbSubParams);
    else
      dbCfg=GWEN_DB_Group_new("config");
    GWEN_DB_SetCharValue(dbCfg, GWEN_DB_FLAGS_OVERWRITE_VARS,
			 "bankCode", localBankCode);
    GWEN_DB_SetCharValue(dbCfg, GWEN_DB_FLAGS_OVERWRITE_VARS,
			 "accountId", localAccountNumber);
    GWEN_DB_SetCharValue(dbCfg, GWEN_DB_FLAGS_OVERWRITE_VARS,
			 "name", localName);

    /* export transactions to DB */
    dbTransfers=GWEN_DB_Group_new("transfers");
    t=AB_ImExporterAccountInfo_GetFirstTransaction(ai);
    while(t) {
      GWEN_DB_NODE *dbTransfer;

      dbTransfer=GWEN_DB_Group_new(groupName);
      AB_Transaction_toDb(t, dbTransfer);
      GWEN_DB_AddGroup(dbTransfers, dbTransfer);
      t=AB_ImExporterAccountInfo_GetNextTransaction(ai);
    }

    /* export transactions to IO */
    rv=GWEN_DBIO_Export(ieh->dbio, sio,
			dbTransfers, dbCfg,
			GWEN_DB_FLAGS_DEFAULT);
    if (rv<0) {
      DBG_ERROR(AQBANKING_LOGDOMAIN, "Error creating DTAUS object");
      GWEN_DB_Group_free(dbTransfers);
      GWEN_DB_Group_free(dbCfg);
      return rv;
    }

    GWEN_DB_Group_free(dbTransfers);
    GWEN_DB_Group_free(dbCfg);
    ai=AB_ImExporterContext_GetNextAccountInfo(ctx);
  } /* while ai */

  return 0;

}







