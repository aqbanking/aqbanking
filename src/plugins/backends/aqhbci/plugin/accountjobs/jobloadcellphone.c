/***************************************************************************
    begin       : Mon Mar 01 2004
    copyright   : (C) 2004-2013 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#ifdef HAVE_CONFIG_H
# include <config.h>
#endif


#include "jobloadcellphone_p.h"
#include "aqhbci_l.h"
#include "accountjob_l.h"
#include "job_l.h"
#include <gwenhywfar/debug.h>
#include <gwenhywfar/misc.h>
#include <gwenhywfar/inherit.h>
#include <gwenhywfar/text.h>

#include <aqbanking/jobloadcellphone_be.h>
#include <aqbanking/job_be.h>

#include <stdlib.h>
#include <assert.h>
#include <string.h>



GWEN_INHERIT(AH_JOB, AH_JOB_LOADCELLPHONE);




/* --------------------------------------------------------------- FUNCTION */
AH_JOB *AH_Job_LoadCellPhone_new(AB_USER *u, AB_ACCOUNT *account) {
  AH_JOB *j;
  AH_JOB_LOADCELLPHONE *aj;
  GWEN_DB_NODE *dbArgs;

  j=AH_AccountJob_new("JobLoadCellPhone", u, account);
  if (!j)
    return 0;

  AH_Job_SetChallengeClass(j, 41);

  GWEN_NEW_OBJECT(AH_JOB_LOADCELLPHONE, aj);
  GWEN_INHERIT_SETDATA(AH_JOB, AH_JOB_LOADCELLPHONE, j, aj,
                       AH_Job_LoadCellPhone_FreeData);
  /* overwrite some virtual functions */
  AH_Job_SetExchangeFn(j, AH_Job_LoadCellPhone_Exchange);

  /* set some known arguments */
  dbArgs=AH_Job_GetArguments(j);
  assert(dbArgs);
  GWEN_DB_SetCharValue(dbArgs, GWEN_DB_FLAGS_DEFAULT,
		       "allAccounts", "N");

  return j;
}



/* --------------------------------------------------------------- FUNCTION */
void GWENHYWFAR_CB AH_Job_LoadCellPhone_FreeData(void *bp, void *p){
  AH_JOB_LOADCELLPHONE *aj;

  aj=(AH_JOB_LOADCELLPHONE*)p;
  GWEN_FREE_OBJECT(aj);
}



/* --------------------------------------------------------------- FUNCTION */
int AH_Job_LoadCellPhone_ExchangeParams(AH_JOB *j, AB_JOB *bj,
					AB_IMEXPORTER_CONTEXT *ctx) {
  AH_JOB_LOADCELLPHONE *aj;
  GWEN_DB_NODE *dbParams;
  GWEN_DB_NODE *dbProduct;
  AB_CELLPHONE_PRODUCT_LIST *pl;
  int i;

  DBG_INFO(AQHBCI_LOGDOMAIN, "Exchanging params");

  assert(j);
  aj=GWEN_INHERIT_GETDATA(AH_JOB, AH_JOB_LOADCELLPHONE, j);
  assert(aj);

  dbParams=AH_Job_GetParams(j);
  DBG_DEBUG(AQHBCI_LOGDOMAIN, "Have this parameters to exchange:");
  if (GWEN_Logger_GetLevel(AQHBCI_LOGDOMAIN)>=GWEN_LoggerLevel_Debug)
    GWEN_DB_Dump(dbParams, 2);

  /* read products */
  pl=AB_CellPhoneProduct_List_new();
  dbProduct=GWEN_DB_FindFirstGroup(dbParams, "product");
  while (dbProduct) {
    AB_CELLPHONE_PRODUCT *cp;
    char numbuf[16];
    const char *s;

    cp=AB_CellPhoneProduct_new();

    i=GWEN_DB_GetIntValue(dbProduct, "Code", 0, -1);
    snprintf(numbuf, sizeof(numbuf), "%d", i);
    AB_CellPhoneProduct_SetId(cp, numbuf);

    i=GWEN_DB_GetIntValue(dbParams, "FreeValueAllowed", 0, 0);
    AB_CellPhoneProduct_SetAllowFreeValue(cp, i);

    s=GWEN_DB_GetCharValue(dbProduct, "Name", 0, NULL);
    if (s)
      AB_CellPhoneProduct_SetProviderName(cp, s);

    s=GWEN_DB_GetCharValue(dbProduct, "ProductName", 0, NULL);
    if (s)
      AB_CellPhoneProduct_SetProductName(cp, s);

    s=GWEN_DB_GetCharValue(dbProduct, "MinimumValue", 0, NULL);
    if (s) {
      AB_VALUE *v;

      v=AB_Value_fromString(s);
      if (v==NULL) {
	DBG_INFO(GWEN_LOGDOMAIN, "Bad minimum value [%s]", s);
      }
      else {
	AB_CellPhoneProduct_SetMinimumValue(cp, v);
	AB_Value_free(v);
      }
    }

    s=GWEN_DB_GetCharValue(dbProduct, "MaximumValue", 0, NULL);
    if (s) {
      AB_VALUE *v;

      v=AB_Value_fromString(s);
      if (v==NULL) {
	DBG_INFO(GWEN_LOGDOMAIN, "Bad maximum value [%s]", s);
      }
      else {
	AB_CellPhoneProduct_SetMaximumValue(cp, v);
	AB_Value_free(v);
      }
    }

    s=GWEN_DB_GetCharValue(dbProduct, "ValueList", 0, NULL);
    if (s) {
      char *ns;
      char *p;
      AB_VALUE_LIST *vl;

      /* read value list */
      vl=AB_CellPhoneProduct_GetValues(cp);
      ns=strdup(s);
      p=ns;
      while(*p) {
	char *p0;

	p0=p;
	p=strchr(p0, ';');
	if (p)
	  *p=0;
	if (*p0) {
	  AB_VALUE *v;

	  v=AB_Value_fromString(p0);
	  if (v) {
	    AB_Value_List_Add(v, vl);
	  }
	  else {
	    DBG_INFO(GWEN_LOGDOMAIN, "Bad value [%s]", p0);
	  }
	}
	if (p)
	  p++;
	else
          break;
      }
    }
    AB_CellPhoneProduct_List_Add(cp, pl);
    dbProduct=GWEN_DB_FindNextGroup(dbProduct, "product");
  }

  AB_JobLoadCellPhone_SetProductList(bj, pl);

  return 0;
}



/* --------------------------------------------------------------- FUNCTION */
int AH_Job_LoadCellPhone_ExchangeArgs(AH_JOB *j, AB_JOB *bj,
				      AB_IMEXPORTER_CONTEXT *ctx) {
  AH_JOB_LOADCELLPHONE *aj;
  GWEN_DB_NODE *dbArgs;
  const AB_CELLPHONE_PRODUCT *cp;
  const AB_VALUE *v;
  const char *s;

  DBG_INFO(AQHBCI_LOGDOMAIN, "Exchanging args");

  assert(j);
  aj=GWEN_INHERIT_GETDATA(AH_JOB, AH_JOB_LOADCELLPHONE, j);
  assert(aj);

  dbArgs=AH_Job_GetArguments(j);

  cp=AB_JobLoadCellPhone_GetCellPhoneProduct(bj);
  if (cp==NULL) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "No product");
    AB_Job_SetStatus(bj, AB_Job_StatusError);
    return GWEN_ERROR_NO_DATA;
  }

  /* cell phone card provider */
  s=AB_CellPhoneProduct_GetId(cp);
  assert(s);
  GWEN_DB_SetCharValue(dbArgs, GWEN_DB_FLAGS_OVERWRITE_VARS, "ProviderCode", s);

  /* phone number */
  s=AB_JobLoadCellPhone_GetPhoneNumber(bj);
  if (s==NULL || *s==0) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "No phone number");
    AB_Job_SetStatus(bj, AB_Job_StatusError);
    return GWEN_ERROR_NO_DATA;
  }
  GWEN_DB_SetCharValue(dbArgs, GWEN_DB_FLAGS_OVERWRITE_VARS, "PhoneNumber", s);

  /* value */
  v=AB_JobLoadCellPhone_GetValue(bj);
  if (v==NULL) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "No value");
    AB_Job_SetStatus(bj, AB_Job_StatusError);
    return GWEN_ERROR_NO_DATA;
  }
  else if (AB_Value_IsZero(v) || AB_Value_IsNegative(v)) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "Bad value");
    AB_Job_SetStatus(bj, AB_Job_StatusError);
    return GWEN_ERROR_INVALID;
  }
  else {
    GWEN_DB_NODE *dbV;
    GWEN_BUFFER *nbuf;
    char *p;
    const char *s;
    int l;
  
    dbV=GWEN_DB_GetGroup(dbArgs, GWEN_DB_FLAGS_OVERWRITE_GROUPS, "value");
    assert(dbV);
  
    nbuf=GWEN_Buffer_new(0, 32, 0, 1);
    if (GWEN_Text_DoubleToBuffer(AB_Value_GetValueAsDouble(v),
				 nbuf)) {
      DBG_ERROR(AQHBCI_LOGDOMAIN, "Buffer overflow");
      GWEN_Buffer_free(nbuf);
      abort();
    }
  
    l=GWEN_Buffer_GetUsedBytes(nbuf);
    if (!l) {
      DBG_ERROR(AQHBCI_LOGDOMAIN, "Error in conversion");
      GWEN_Buffer_free(nbuf);
      abort();
    }
  
    /* replace "C" comma with "DE" comma, remove thousand's comma */
    p=GWEN_Buffer_GetStart(nbuf);
    s=p;
    while(*s) {
      if (*s=='.') {
	*p=',';
	p++;
      }
      else if (*s!=',') {
	*p=*s;
	p++;
      }
      s++;
    } /* while */
    *p=0;
  
    if (strchr(GWEN_Buffer_GetStart(nbuf), ',')) {
      /* kill all trailing '0' behind the comma */
      p=GWEN_Buffer_GetStart(nbuf)+l;
      while(l--) {
	--p;
	if (*p=='0')
	  *p=0;
	else
	  break;
      }
    }
    else
      GWEN_Buffer_AppendString(nbuf, ",");
  
    /* store value */
    GWEN_DB_SetCharValue(dbV, GWEN_DB_FLAGS_OVERWRITE_VARS,
			 "value",
			 GWEN_Buffer_GetStart(nbuf));
    GWEN_Buffer_free(nbuf);
  
    s=AB_Value_GetCurrency(v);
    if (!s)
      s="EUR";
    GWEN_DB_SetCharValue(dbV, GWEN_DB_FLAGS_OVERWRITE_VARS,
			 "currency", s);
  }

  return 0;
}



/* --------------------------------------------------------------- FUNCTION */
int AH_Job_LoadCellPhone_Exchange(AH_JOB *j, AB_JOB *bj,
				  AH_JOB_EXCHANGE_MODE m,
				  AB_IMEXPORTER_CONTEXT *ctx){
  AH_JOB_LOADCELLPHONE *aj;

  DBG_INFO(AQHBCI_LOGDOMAIN, "Exchanging (%d)", m);

  assert(j);
  aj=GWEN_INHERIT_GETDATA(AH_JOB, AH_JOB_LOADCELLPHONE, j);
  assert(aj);

  if (AB_Job_GetType(bj)!=AB_Job_TypeLoadCellPhone) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "Not a GetLoadCellPhone job");
    return GWEN_ERROR_INVALID;
  }

  switch(m) {
  case AH_Job_ExchangeModeParams:
    return AH_Job_LoadCellPhone_ExchangeParams(j, bj, ctx);
  case AH_Job_ExchangeModeArgs:
    return AH_Job_LoadCellPhone_ExchangeArgs(j, bj, ctx);
  case AH_Job_ExchangeModeResults:
    return 0;
  default:
    DBG_NOTICE(AQHBCI_LOGDOMAIN, "Unsupported exchange mode");
    return GWEN_ERROR_NOT_SUPPORTED;
  } /* switch */
}










