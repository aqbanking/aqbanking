/***************************************************************************
    begin       : Mon Mar 01 2004
    copyright   : (C) 2018 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "swift_p.h"
#include "aqbanking/i18n_l.h"

#include <aqbanking/banking.h>
#include <aqbanking/types/balance.h>

#include <gwenhywfar/debug.h>
#include <gwenhywfar/misc.h>
#include <gwenhywfar/gui.h>
#include <gwenhywfar/inherit.h>


/*#define SWIFT_VERBOSE_DEBUG*/


GWEN_INHERIT(AB_IMEXPORTER, AH_IMEXPORTER_SWIFT);



AB_IMEXPORTER *AB_ImExporterSWIFT_new(AB_BANKING *ab)
{
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



void GWENHYWFAR_CB AH_ImExporterSWIFT_FreeData(void *bp, void *p)
{
  AH_IMEXPORTER_SWIFT *ieh;

  ieh=(AH_IMEXPORTER_SWIFT *)p;
  GWEN_DBIO_free(ieh->dbio);
  GWEN_FREE_OBJECT(ieh);
}



int AH_ImExporterSWIFT_Import(AB_IMEXPORTER *ie,
                              AB_IMEXPORTER_CONTEXT *ctx,
                              GWEN_SYNCIO *sio,
                              GWEN_DB_NODE *params)
{
  AH_IMEXPORTER_SWIFT *ieh;
  GWEN_DB_NODE *dbData;
  GWEN_DB_NODE *dbSubParams;
  int rv;

  assert(ie);
  ieh=GWEN_INHERIT_GETDATA(AB_IMEXPORTER, AH_IMEXPORTER_SWIFT, ie);
  assert(ieh);
  assert(ieh->dbio);

  dbSubParams=GWEN_DB_GetGroup(params, GWEN_PATH_FLAGS_NAMEMUSTEXIST, "params");
  dbData=GWEN_DB_Group_new("transactions");
  GWEN_Gui_ProgressLog(0, GWEN_LoggerLevel_Debug,
                       I18N("Reading file..."));

#ifdef SWIFT_VERBOSE_DEBUG
  if (params) {
    DBG_ERROR(0, "Parameters for SWIFT-Parser:");
    GWEN_DB_Dump(params, 2);
  }
#endif


  DBG_INFO(AQBANKING_LOGDOMAIN, "Importing SWIFT data into GWEN_DB");
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
  DBG_INFO(AQBANKING_LOGDOMAIN, "Importing SWIFT data into GWEN_DB: done");

#ifdef SWIFT_VERBOSE_DEBUG
  DBG_ERROR(0, "Imported SWIFT data is (GWEN_DB):");
  GWEN_DB_Dump(dbData, 2);
#endif

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
                                        GWEN_DB_NODE *dbParams)
{
  GWEN_DB_NODE *dbT;
  uint32_t progressId;

  DBG_INFO(AQBANKING_LOGDOMAIN, "Importing from DB group \"%s\"", GWEN_DB_GroupName(db));

  progressId=GWEN_Gui_ProgressStart(GWEN_GUI_PROGRESS_DELAY |
                                    GWEN_GUI_PROGRESS_ALLOW_EMBED |
                                    GWEN_GUI_PROGRESS_SHOW_PROGRESS |
                                    GWEN_GUI_PROGRESS_SHOW_ABORT,
                                    I18N("Importing parsed data..."),
                                    NULL,
                                    GWEN_DB_Groups_Count(db),
                                    0);
  dbT=GWEN_DB_GetFirstGroup(db);
  while (dbT) {
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
      const char *s;

      t=AB_Transaction_fromDb(dbT);
      if (!t) {
        DBG_ERROR(AQBANKING_LOGDOMAIN, "Error in config file");
        GWEN_Gui_ProgressLog(0, GWEN_LoggerLevel_Error,
                             I18N("Error in config file"));
        return GWEN_ERROR_GENERIC;
      }

      /* some translations */
      s=AB_Transaction_GetRemoteIban(t);
      if (!(s && *s)) {
        const char *sAid;

        /* no remote IBAN set, check whether the bank sends this info in the
         * fields for national account specifications (instead of the SWIFT
         * field "?38" which was specified for this case) */
        sAid=AB_Transaction_GetRemoteAccountNumber(t);
        if (sAid && *sAid && AB_Banking_CheckIban(sAid)==0) {
          /* there is a remote account number specification, and that is an IBAN,
           * so we set that accordingly */
          DBG_INFO(AQBANKING_LOGDOMAIN, "Setting remote IBAN from account number");
          AB_Transaction_SetRemoteIban(t, sAid);

          /* set remote BIC if it not already is */
          s=AB_Transaction_GetRemoteBic(t);
          if (!(s && *s)) {
            const char *sBid;

            sBid=AB_Transaction_GetRemoteBankCode(t);
            if (sBid && *sBid) {
              DBG_INFO(AQBANKING_LOGDOMAIN, "Setting remote BIC from bank code");
              AB_Transaction_SetRemoteBic(t, sBid);
            }
          }
        }
      }

#if 0 /* disabled, will be removed later */

      /* ABWA+: replace remote name with ABWA+ content */
      s=GWEN_DB_GetCharValue(dbT, "sepa/ABWA", 0, NULL);
      if (s && *s) {
        int i;

        //DBG_ERROR(AQBANKING_LOGDOMAIN, "Replacing remote name with ABWA value [%s]", s);
        for (i=0; i<2; i++) {
          s=GWEN_DB_GetCharValue(dbT, "sepa/ABWA", i, NULL);
          if (s && *s) {
            AB_Transaction_SetRemoteName(t, s);
          }
        }
      }
#endif

      /* read all lines of the remote name and concatenate them (addresses bug #57) */
      if (1) {
	const char *varName;
	int i;
	GWEN_BUFFER *nameBuf;

	if (GWEN_DB_VariableExists(dbT, "sepa/ABWA"))
	  varName="sepa/ABWA";
	else
	  varName="remoteName";

	nameBuf=GWEN_Buffer_new(0, 256, 0, 1);
	for (i=0; i<2; i++) {
	  s=GWEN_DB_GetCharValue(dbT, varName, i, NULL);
	  if (s && *s) {
#if 0 /* TODO: add separator? */
	    if (i>0)
	      GWEN_Buffer_AppendString(nameBuf, " ");
#endif
	    GWEN_Buffer_AppendString(nameBuf, s);
	  }
	}
	if (GWEN_Buffer_GetUsedBytes(nameBuf))
	  AB_Transaction_SetRemoteName(t, GWEN_Buffer_GetStart(nameBuf));
      }


      /* ABWE+: replace local name with ABWE+ content */
      s=GWEN_DB_GetCharValue(dbT, "sepa/ABWE", 0, NULL);
      if (s && *s) {
        AB_Transaction_SetLocalName(t, s);
      }


      /* add transaction */
      DBG_DEBUG(AQBANKING_LOGDOMAIN, "Adding transaction");
      GWEN_Gui_ProgressLog(0, GWEN_LoggerLevel_Debug, I18N("Adding transaction"));
      AB_ImExporterContext_AddTransaction(ctx, t);
    }
    else if (strcasecmp(GWEN_DB_GroupName(dbT), "startSaldo")==0) {
      /* ignore start saldo, but since the existence of this group shows
       * that we in fact are within a swift DB group we don't need recursions.
       */
    }
    else if (strcasecmp(GWEN_DB_GroupName(dbT), "endSaldo")==0) {
      GWEN_DB_NODE *dbX;
      GWEN_DATE *dt=0;
      const char *s;
      const char *bankCode;
      const char *accountNumber;
      const char *iban;

      bankCode=GWEN_DB_GetCharValue(dbT, "localBankCode", 0, 0);
      accountNumber=GWEN_DB_GetCharValue(dbT, "localAccountNumber", 0, 0);
      iban=GWEN_DB_GetCharValue(dbT, "localIban", 0, 0);

      /* read date */
      s=GWEN_DB_GetCharValue(dbT, "date", 0, NULL);
      if (s && *s) {
        dt=GWEN_Date_fromString(s);
        if (dt==NULL) {
          DBG_ERROR(AQBANKING_LOGDOMAIN, "Bad date in saldo");
        }
      }

      dbX=GWEN_DB_GetGroup(dbT, GWEN_PATH_FLAGS_NAMEMUSTEXIST, "value");
      if (dbX) {
        AB_VALUE *v;

        v=AB_Value_fromDb(dbX);
        if (v) {
          AB_BALANCE *bal;
          AB_IMEXPORTER_ACCOUNTINFO *iea;

          bal=AB_Balance_new();
          AB_Balance_SetDate(bal, dt);
          AB_Balance_SetValue(bal, v);
          AB_Value_free(v);

          /* determine saldo type */
          s=GWEN_DB_GetCharValue(dbT, "type", 0, NULL);
          if (s && *s && strcasecmp(s, "final")==0)
            AB_Balance_SetType(bal, AB_Balance_TypeNoted); /* TODO: maybe use "booked" here? */
          else
            AB_Balance_SetType(bal, AB_Balance_TypeTemporary);

          iea=AB_ImExporterContext_GetOrAddAccountInfo(ctx, 0, iban, bankCode, accountNumber, 0);
          DBG_DEBUG(AQBANKING_LOGDOMAIN, "Adding balance");
          AB_ImExporterAccountInfo_AddBalance(iea, bal);
        }
      }
      GWEN_Date_free(dt);
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
  DBG_INFO(AQBANKING_LOGDOMAIN, "Importing from DB group \"%s\": Done", GWEN_DB_GroupName(db));
  return 0;
}



int AH_ImExporterSWIFT_CheckFile(AB_IMEXPORTER *ie, const char *fname)
{
  AH_IMEXPORTER_SWIFT *ieh;
  GWEN_DBIO_CHECKFILE_RESULT rv;

  assert(ie);
  ieh=GWEN_INHERIT_GETDATA(AB_IMEXPORTER, AH_IMEXPORTER_SWIFT, ie);
  assert(ieh);
  assert(ieh->dbio);

  rv=GWEN_DBIO_CheckFile(ieh->dbio, fname);
  switch (rv) {
  case GWEN_DBIO_CheckFileResultOk:
    return 0;
  case GWEN_DBIO_CheckFileResultNotOk:
    return GWEN_ERROR_BAD_DATA;
  case GWEN_DBIO_CheckFileResultUnknown:
    return AB_ERROR_INDIFFERENT;
  default:
    return GWEN_ERROR_GENERIC;
  } /* switch */
}







