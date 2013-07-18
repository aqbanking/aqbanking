/***************************************************************************
    begin       : Sat Nov 29 2008
    copyright   : (C) 2008 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#ifdef HAVE_CONFIG_H
# include <config.h>
#endif


#include "jobsinglesepa_p.h"
#include "aqhbci_l.h"
#include "accountjob_l.h"
#include "provider_l.h"
#include <aqhbci/account.h>

#include <gwenhywfar/debug.h>
#include <gwenhywfar/misc.h>
#include <gwenhywfar/inherit.h>
#include <gwenhywfar/dbio.h>
#include <gwenhywfar/text.h>

#include <aqbanking/jobsepatransfer_be.h>
#include <aqbanking/jobsepadebitnote_be.h>
#include <aqbanking/job_be.h>

#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>


GWEN_INHERIT(AH_JOB, AH_JOB_SINGLESEPA);



AH_JOB *AH_Job_SingleSepaBase_new(AB_USER *u,
				  AB_ACCOUNT *account,
				  AB_JOB_TYPE jobType) {
  AH_JOB *j;
  AH_JOB_SINGLESEPA *aj;
  //GWEN_DB_NODE *dbArgs;

  switch(jobType) {
  case AB_Job_TypeSepaTransfer:
    j=AH_AccountJob_new("JobSepaSingleTransfer", u, account);
    break;
  case AB_Job_TypeSepaDebitNote:
    j=AH_AccountJob_new("JobSepaSingleDebitNote", u, account);
    break;
  default:
    DBG_ERROR(AQHBCI_LOGDOMAIN, "Unknown job type %d", jobType);
    j=0;
  }

  if (!j)
    return NULL;

  GWEN_NEW_OBJECT(AH_JOB_SINGLESEPA, aj);
  GWEN_INHERIT_SETDATA(AH_JOB, AH_JOB_SINGLESEPA, j, aj,
                       AH_Job_SingleSepa_FreeData);
  aj->jobType=jobType;
  /* overwrite some virtual functions */
  AH_Job_SetProcessFn(j, AH_Job_SingleSepa_Process);
  AH_Job_SetExchangeFn(j, AH_Job_SingleSepa_Exchange);

  /* preset some arguments */
  //dbArgs=
  AH_Job_GetArguments(j);

  switch(jobType) {
  case AB_Job_TypeSepaTransfer:
    break;
  case AB_Job_TypeSepaDebitNote:
    break;
  default:
    DBG_ERROR(AQHBCI_LOGDOMAIN, "Unknown job type %d", jobType);
    abort();
  }

  return j;
}


void GWENHYWFAR_CB AH_Job_SingleSepa_FreeData(void *bp, void *p) {
  AH_JOB_SINGLESEPA *aj;

  aj=(AH_JOB_SINGLESEPA*)p;
  free(aj->fiid);
  free(aj->oldFiid);

  GWEN_FREE_OBJECT(aj);
}



int AH_Job_SingleSepa_Process(AH_JOB *j, AB_IMEXPORTER_CONTEXT *ctx) {
  AH_JOB_SINGLESEPA *aj;
  GWEN_DB_NODE *dbResponses;
  GWEN_DB_NODE *dbCurr;

  assert(j);
  aj=GWEN_INHERIT_GETDATA(AH_JOB, AH_JOB_SINGLESEPA, j);
  assert(aj);
  DBG_INFO(AQHBCI_LOGDOMAIN, "Processing %s",
           AB_Job_Type2Char(aj->jobType));
  dbResponses=AH_Job_GetResponses(j);
  assert(dbResponses);

  switch(aj->jobType) {
  case AB_Job_TypeSepaTransfer:
  case AB_Job_TypeSepaDebitNote:
    dbCurr=GWEN_DB_GetFirstGroup(dbResponses);
    while(dbCurr) {
      int rv;
  
      rv=AH_Job_CheckEncryption(j, dbCurr);
      if (rv) {
	DBG_INFO(AQHBCI_LOGDOMAIN, "Compromised security (encryption)");
	AH_Job_SetStatus(j, AH_JobStatusError);
	return rv;
      }
      rv=AH_Job_CheckSignature(j, dbCurr);
      if (rv) {
	DBG_INFO(AQHBCI_LOGDOMAIN, "Compromised security (signature)");
	AH_Job_SetStatus(j, AH_JobStatusError);
	return rv;
      }
      dbCurr=GWEN_DB_GetNextGroup(dbCurr);
    }
    break;

  default:
    DBG_ERROR(AQHBCI_LOGDOMAIN,
              "Unhandled job type %d", aj->jobType);
    return GWEN_ERROR_INVALID;
  }

  return 0;
}



int AH_Job_SingleSepa_Exchange(AH_JOB *j, AB_JOB *bj,
			       AH_JOB_EXCHANGE_MODE m,
			       AB_IMEXPORTER_CONTEXT *ctx) {
  AH_JOB_SINGLESEPA *aj;
  AB_BANKING *ab;
  AB_USER *u;

  DBG_INFO(AQHBCI_LOGDOMAIN, "Exchanging (%d)", m);

  assert(j);
  aj=GWEN_INHERIT_GETDATA(AH_JOB, AH_JOB_SINGLESEPA, j);
  assert(aj);

  ab=AH_Job_GetBankingApi(j);
  assert(ab);

  if (aj->jobType!=AB_Job_GetType(bj)) {
    DBG_ERROR(AQHBCI_LOGDOMAIN,
              "Different job types");
    return GWEN_ERROR_INVALID;
  }

  u=AH_Job_GetUser(j);
  assert(u);

  switch(m) {
  case AH_Job_ExchangeModeParams: {
    AB_TRANSACTION_LIMITS *lim;

    /* set some default limits */
    lim=AB_TransactionLimits_new();
    AB_TransactionLimits_SetMaxLenPurpose(lim, 27);
    AB_TransactionLimits_SetMaxLenRemoteName(lim, 27);
    AB_TransactionLimits_SetMaxLinesRemoteName(lim, 1);
    AB_TransactionLimits_SetMaxLinesPurpose(lim, 2);

    switch(aj->jobType) {
    case AB_Job_TypeSepaTransfer:
      AB_JobSepaTransfer_SetFieldLimits(bj, lim);
      break;
    case AB_Job_TypeSepaDebitNote:
      AB_JobSepaDebitNote_SetFieldLimits(bj, lim);
      break;
    default:
      DBG_ERROR(AQHBCI_LOGDOMAIN, "Unknown job type %d", aj->jobType);
      AB_TransactionLimits_free(lim);
      return GWEN_ERROR_INVALID;
      break;
    }
    AB_TransactionLimits_free(lim);

    break;
  }

  case AH_Job_ExchangeModeArgs: {
    const AB_TRANSACTION_LIMITS *lim=NULL;
    AB_TRANSACTION *t=NULL;
    int rv;
    GWEN_DB_NODE *dbArgs;
    const char *profileName="";
    const char *descriptor="";
    const char *s;

    dbArgs=AH_Job_GetArguments(j);

    /* get limits and transaction */
    switch(aj->jobType) {
    case AB_Job_TypeSepaTransfer:
      lim=AB_JobSepaTransfer_GetFieldLimits(bj);
      t=AB_JobSepaTransfer_GetTransaction(bj);
      /* choose from HISPAS */
      /* first check for any descriptor for pain 001.002.03 */
      s=AH_User_FindSepaDescriptor(u, "*001.002.03*");
      if (s) {
        profileName="001_002_03";
        descriptor=s;
      }
      else {
        /* look for pain 001.001.02 */
        s=AH_User_FindSepaDescriptor(u, "*001.001.02*");
        if (s) {
          profileName="ccm";
          descriptor=s;
        }
      }

      /* check for valid descriptor */
      if (!(descriptor && *descriptor)) {
        DBG_ERROR(AQHBCI_LOGDOMAIN, "No SEPA descriptor found, please update your SEPA account information");
        return GWEN_ERROR_GENERIC;
      }
      DBG_INFO(AQHBCI_LOGDOMAIN, "Using SEPA descriptor %s", descriptor);
      break;
    case AB_Job_TypeSepaDebitNote:
      lim=AB_JobSepaDebitNote_GetFieldLimits(bj);
      t=AB_JobSepaDebitNote_GetTransaction(bj);
      /* profileName="ccm"; insert correct name */
      /* descriptor="pain.001.001.02"; insert correct name */
      break;
    default:
      DBG_ERROR(AQHBCI_LOGDOMAIN, "Unknown job type %d", aj->jobType);
      return GWEN_ERROR_INVALID;
      break;
    }

    if (t==NULL) {
      DBG_ERROR(AQHBCI_LOGDOMAIN, "No transaction in job");
      return GWEN_ERROR_INVALID;
    }

    /* validate transaction */
    rv=AH_Provider_ValidateTransfer(t, bj, lim);
    if (rv<0) {
      DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
      return rv;
    }
    else {
      AB_IMEXPORTER_CONTEXT *ioc;
      AB_TRANSACTION *cpy;
      GWEN_BUFFER *dbuf;
      GWEN_TIME *ti;

      ioc=AB_ImExporterContext_new();
      cpy=AB_Transaction_dup(t);
      /* don't set date, SEPA exporter will create NODATE (1999/01/01) */
      AB_ImExporterContext_AddTransaction(ioc, cpy);

      dbuf=GWEN_Buffer_new(0, 256, 0, 1);
      rv=AB_Banking_ExportToBuffer(ab, ioc, "sepa", profileName, dbuf);
      AB_ImExporterContext_free(ioc);
      if (rv<0) {
	DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
	GWEN_Buffer_free(dbuf);
	return rv;
      }

      /* store descriptor */
      GWEN_DB_SetCharValue(dbArgs,
			   GWEN_DB_FLAGS_OVERWRITE_VARS,
			   "descriptor",
			   descriptor);
      /* store transfer */
      GWEN_DB_SetBinValue(dbArgs,
			  GWEN_DB_FLAGS_OVERWRITE_VARS,
			  "transfer",
			  GWEN_Buffer_GetStart(dbuf),
			  GWEN_Buffer_GetUsedBytes(dbuf));
      GWEN_Buffer_free(dbuf);
    }

    break;
  }

  case AH_Job_ExchangeModeResults: {
    break;
  }

  default:
    DBG_NOTICE(AQHBCI_LOGDOMAIN, "Unsupported exchange mode");
    return GWEN_ERROR_NOT_SUPPORTED;
  } /* switch */

  return 0;
}



AH_JOB *AH_Job_SingleSepaTransfer_new(AB_USER *u, AB_ACCOUNT *account) {
  return AH_Job_SingleSepaBase_new(u, account, AB_Job_TypeSepaTransfer);
}



AH_JOB *AH_Job_SingleSepaDebitNote_new(AB_USER *u, AB_ACCOUNT *account) {
  return AH_Job_SingleSepaBase_new(u, account, AB_Job_TypeSepaDebitNote);
}



const char *AH_Job_SingleSepaTransfer_GetFiid(AH_JOB *j) {
  AH_JOB_SINGLESEPA *aj;

  assert(j);
  aj=GWEN_INHERIT_GETDATA(AH_JOB, AH_JOB_SINGLESEPA, j);
  assert(aj);

  return aj->fiid;
}



const char *AH_Job_SingleSepaTransfer_GetOldFiid(AH_JOB *j) {
  AH_JOB_SINGLESEPA *aj;

  assert(j);
  aj=GWEN_INHERIT_GETDATA(AH_JOB, AH_JOB_SINGLESEPA, j);
  assert(aj);

  return aj->oldFiid;
}







