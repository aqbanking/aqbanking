/***************************************************************************
    begin       : Sat Dec 01 2018
    copyright   : (C) 2022 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "aqpaypal/provider_getbalance.h"

#include "aqpaypal/provider_request.h"

#include <gwenhywfar/debug.h>
#include <gwenhywfar/gui.h>
#include <gwenhywfar/i18n.h>


#define I18N(msg) GWEN_I18N_Translate(PACKAGE, msg)





int APY_Provider_ExecGetBal(AB_PROVIDER *pro,
                            AB_IMEXPORTER_ACCOUNTINFO *ai,
                            AB_USER *u,
                            AB_TRANSACTION *j)
{
  GWEN_BUFFER *tbuf;
  int rv;
  GWEN_DB_NODE *dbResponse;
  GWEN_DB_NODE *dbCurr;

  tbuf=GWEN_Buffer_new(0, 256, 0, 1);
  rv=APY_Provider_SetupUrlString(pro, u, tbuf);
  if (rv<0) {
    DBG_INFO(AQPAYPAL_LOGDOMAIN, "here (%d)", rv);
    GWEN_Buffer_free(tbuf);
    AB_Transaction_SetStatus(j, AB_Transaction_StatusError);
    return rv;
  }

  GWEN_Buffer_AppendString(tbuf, "&method=GetBalance");

  /* send and receive */
  AB_Transaction_SetStatus(j, AB_Transaction_StatusSending);
  dbResponse=APY_Provider_SendRequestParseResponse(pro, u, GWEN_Buffer_GetStart(tbuf), "getBalance");
  GWEN_Buffer_free(tbuf);
  if (dbResponse==NULL) {
    DBG_INFO(AQPAYPAL_LOGDOMAIN, "here");
    AB_Transaction_SetStatus(j, AB_Transaction_StatusError);
    return GWEN_ERROR_GENERIC;
  }


  /* handle response */
  AB_Transaction_SetStatus(j, AB_Transaction_StatusAccepted);
  dbCurr=GWEN_DB_GetFirstGroup(dbResponse);
  while (dbCurr) {
    AB_BALANCE *bal;
    GWEN_DATE *t=NULL;
    AB_VALUE *vc;
    const char *p;

    DBG_NOTICE(AQPAYPAL_LOGDOMAIN, "Got a balance");

    /* read and parse value */
    p=GWEN_DB_GetCharValue(dbCurr, "L_AMT", 0, 0);
    if (!p)
      return GWEN_ERROR_BAD_DATA;
    vc=AB_Value_fromString(p);
    if (vc==NULL)
      return GWEN_ERROR_BAD_DATA;

    /* read currency (if any) */
    p=GWEN_DB_GetCharValue(dbCurr, "L_CURRENCYCODE", 0, "EUR");
    if (p)
      AB_Value_SetCurrency(vc, p);

    p=GWEN_DB_GetCharValue(dbResponse, "TIMESTAMP", 0, NULL);
    if (p && *p) {
      /*t=GWEN_Time_fromUtcString(p, "YYYY-MM-DDThh:mm:ssZ");*/
      t=GWEN_Date_fromStringWithTemplate(p, "YYYY-MM-DD");
      if (t==NULL) {
        DBG_ERROR(AQPAYPAL_LOGDOMAIN, "Invalid timespec [%s]", p);
      }
    }
    else {
      DBG_ERROR(AQPAYPAL_LOGDOMAIN, "Missing timespec");
    }

    bal=AB_Balance_new();
    AB_Balance_SetType(bal, AB_Balance_TypeBooked);
    AB_Balance_SetDate(bal, t);
    AB_Balance_SetValue(bal, vc);

    AB_Value_free(vc);
    GWEN_Date_free(t);

    /* add new balance */
    AB_ImExporterAccountInfo_AddBalance(ai, bal);
    break; /* break loop, we found the balance */

    dbCurr=GWEN_DB_GetNextGroup(dbCurr);
  }

  GWEN_DB_Group_free(dbResponse);
  AB_Transaction_SetStatus(j, AB_Transaction_StatusAccepted);

  return 0;
}

