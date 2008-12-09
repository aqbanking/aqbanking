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

  switch(jobType) {
  case AB_Job_TypeSepaTransfer
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
    return 0;

  GWEN_NEW_OBJECT(AH_JOB_SINGLETRANSFER, aj);
  GWEN_INHERIT_SETDATA(AH_JOB, AH_JOB_SINGLESEPA, j, aj,
                       AH_Job_SingleSepa_FreeData);
  aj->jobType=jobType;
  /* overwrite some virtual functions */
  AH_Job_SetProcessFn(j, AH_Job_SingleSepa_Process);
  AH_Job_SetExchangeFn(j, AH_Job_SingleSepa_Exchange);

  return j;
}


void GWENHYWFAR_CB AH_Job_SingleSepa_FreeData(void *bp, void *p) {
  AH_JOB_SINGLESEPA *aj;

  aj=(AH_JOB_SINGLESEPA*)p;
  free(aj->fiid);
  free(aj->oldFiid);

  GWEN_FREE_OBJECT(aj);
}



int AH_Job_SingleSepa_Process(AH_JOB *j,
			      AB_IMEXPORTER_CONTEXT *ctx,
			      uint32_t guiid) {
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
			       AB_IMEXPORTER_CONTEXT *ctx,
			       uint32_t guiid) {
  AH_JOB_SINGLESEPA *aj;

  DBG_INFO(AQHBCI_LOGDOMAIN, "Exchanging (%d)", m);

  assert(j);
  aj=GWEN_INHERIT_GETDATA(AH_JOB, AH_JOB_SINGLESEPA, j);
  assert(aj);

  if (aj->jobType!=AB_Job_GetType(bj)) {
    DBG_ERROR(AQHBCI_LOGDOMAIN,
              "Different job types");
    return GWEN_ERROR_INVALID;
  }

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
    case AB_Job_TypeSepaTransfer
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
    break;
  }

  case AH_Job_ExchangeModeResults: {
    break;
  }

  default:
    DBG_NOTICE(AQHBCI_LOGDOMAIN, "Unsupported exchange mode");
    return GWEN_ERROR_NOT_SUPPORTED;
  } /* switch */

}



int AH_Job_SingleSepa__ValidateTransfer(AB_JOB *bj,
					AH_JOB *mj,
					AB_TRANSACTION *t) {
}




AH_JOB *AH_Job_SingleSepaTransfer_new(AB_USER *u, AB_ACCOUNT *account) {
}



AH_JOB *AH_Job_SingleSepaDebitNote_new(AB_USER *u, AB_ACCOUNT *account) {
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







