/***************************************************************************
 $RCSfile$
 -------------------
 cvs         : $Id$
 begin       : Mon Mar 01 2004
 copyright   : (C) 2004 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 * This file is part of the project "AqBanking".                           *
 * Please see toplevel file COPYING of that project for license details.   *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "transaction_p.h"
#include <aqbanking/transactionfns.h>
#include <gwenhywfar/bio_buffer.h>
#include <gwenhywfar/debug.h>




int AB_Transaction_Compare(const AB_TRANSACTION *t1,
                           const AB_TRANSACTION *t0) {
  if (t1==t0)
    return 0;

  if (t1 && t0) {
    GWEN_DB_NODE *dbT;
    GWEN_BUFFER *buf1;
    GWEN_BUFFER *buf0;

    buf1=GWEN_Buffer_new(0, 256, 0, 1);
    buf0=GWEN_Buffer_new(0, 256, 0, 1);

    /* prepare first buffer */
    dbT=GWEN_DB_Group_new("transaction");
    if (AB_Transaction_toDb(t1, dbT)==0) {
      int err;

      /* remove variables from comparison */
      GWEN_DB_DeleteVar(dbT, "status");

      err=GWEN_DB_WriteToBuffer(dbT, buf1, GWEN_DB_FLAGS_COMPACT);
      if (err) {
	DBG_ERROR(AQBANKING_LOGDOMAIN,
		  "INTERNAL: Error writing DB to buffer");
	GWEN_Buffer_free(buf0);
	GWEN_Buffer_free(buf1);
	GWEN_DB_Group_free(dbT);
	return -1;
      }
    }
    GWEN_DB_Group_free(dbT);
  
    /* prepare second buffer */
    dbT=GWEN_DB_Group_new("transaction");
    if (AB_Transaction_toDb(t0, dbT)==0) {
      int err;

      /* remove variables from comparison */
      GWEN_DB_DeleteVar(dbT, "status");

      err=GWEN_DB_WriteToBuffer(dbT, buf0, GWEN_DB_FLAGS_COMPACT);
      if (err) {
	DBG_ERROR(AQBANKING_LOGDOMAIN,
		  "INTERNAL: Error writing DB to buffer");
	GWEN_Buffer_free(buf0);
	GWEN_Buffer_free(buf1);
	GWEN_DB_Group_free(dbT);
	return -1;
      }
    }
    GWEN_DB_Group_free(dbT);
  
    /* actually compare */
    if (strcasecmp(GWEN_Buffer_GetStart(buf1),
                   GWEN_Buffer_GetStart(buf0))==0) {
      GWEN_Buffer_free(buf0);
      GWEN_Buffer_free(buf1);
      return 0;
    }
    GWEN_Buffer_free(buf0);
    GWEN_Buffer_free(buf1);
  }

  return 1;
}


void AB_Transaction_FillLocalFromAccount(AB_TRANSACTION *t, const AB_ACCOUNT *a)
{
  const char *s;

  assert(t);
  assert(a);

  /* local account */
  s=AB_Account_GetCountry(a);
  if (!s || !*s)
    s="de";
  AB_Transaction_SetLocalCountry(t, s);
  AB_Transaction_SetRemoteCountry(t, s);

  s=AB_Account_GetBankCode(a);
  if (s && *s)
    AB_Transaction_SetLocalBankCode(t, s);
  s=AB_Account_GetAccountNumber(a);
  if (s && *s)
    AB_Transaction_SetLocalAccountNumber(t, s);
  s=AB_Account_GetOwnerName(a);
  if (s && *s)
    AB_Transaction_SetLocalName(t, s);
}

