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

#include "provider_job.h"

#include "jobgetbalance_l.h"
#include "jobgettransactions_l.h"
#include "jobgettrans_camt_l.h"
#include "jobloadcellphone_l.h"
#include "jobsepaxfersingle_l.h"
#include "jobsepaxfermulti_l.h"
#include "jobsepadebitdatedsinglecreate_l.h"
#include "jobsepadebitdatedmulticreate_l.h"
#include "jobsepacor1datedsinglecreate_l.h"
#include "jobsepacor1datedmulticreate_l.h"

#include "jobsepastandingorderdelete_l.h"
#include "jobsepastandingordercreate_l.h"
#include "jobsepastandingordermodify_l.h"
#include "jobsepastandingorderget_l.h"
#include "jobgetestatements_l.h"
#include "jobgetestatements2_l.h"
#include "jobgetdepot_l.h"

#include "jobsepadebitsingle_l.h" /* deprecated job */




int AH_Provider_CreateHbciJob(AB_PROVIDER *pro, AB_USER *mu, AB_ACCOUNT *ma, int cmd, AH_JOB **pHbciJob)
{
  AH_JOB *mj;
  uint32_t aFlags;

  assert(pro);

  aFlags=AH_Account_GetFlags(ma);

  mj=0;
  switch (cmd) {

  case AB_Transaction_CommandGetBalance:
    mj=AH_Job_GetBalance_new(pro, mu, ma);
    if (!mj) {
      DBG_INFO(AQHBCI_LOGDOMAIN, "Job not supported with this account");
      return GWEN_ERROR_NOT_AVAILABLE;
    }
    break;

  case AB_Transaction_CommandGetTransactions:
    if (aFlags & AH_BANK_FLAGS_PREFER_CAMT_DOWNLOAD) {
      DBG_INFO(AQHBCI_LOGDOMAIN, "Customer prefers CAMT download");
      mj=AH_Job_GetTransactionsCAMT_new(pro, mu, ma);
      if (!mj) {
        DBG_WARN(AQHBCI_LOGDOMAIN, "CAMT download job not supported with this account, falling back to SWIFT");
      }
    }
    if (!mj) {
      mj=AH_Job_GetTransactions_new(pro, mu, ma);
      if (!mj) {
        DBG_INFO(AQHBCI_LOGDOMAIN, "Job not supported with this account");
        return GWEN_ERROR_NOT_AVAILABLE;
      }
    }
    break;

  case AB_Transaction_CommandLoadCellPhone:
    mj=AH_Job_LoadCellPhone_new(pro, mu, ma);
    if (!mj) {
      DBG_INFO(AQHBCI_LOGDOMAIN, "Job not supported with this account");
      return GWEN_ERROR_NOT_AVAILABLE;
    }
    break;

  case AB_Transaction_CommandSepaTransfer:
    if (!(aFlags & AH_BANK_FLAGS_SEPA_PREFER_SINGLE_TRANSFER)) {
      DBG_INFO(AQHBCI_LOGDOMAIN, "Customer prefers multi jobs");

      /* try multi transfer first */
      mj=AH_Job_SepaTransferMulti_new(pro, mu, ma);
      if (!mj) {
        DBG_WARN(AQHBCI_LOGDOMAIN, "Job \"SepaTransferMulti\" not supported with this account");

        /* try single transfer */
        mj=AH_Job_SepaTransferSingle_new(pro, mu, ma);
        if (!mj) {
          DBG_WARN(AQHBCI_LOGDOMAIN, "Job \"SepaTransferSingle\" not supported with this account");
          return GWEN_ERROR_NOT_AVAILABLE;
        }
      }
    }
    else {
      /* try single job first */
      mj=AH_Job_SepaTransferSingle_new(pro, mu, ma);
      if (!mj) {
        DBG_WARN(AQHBCI_LOGDOMAIN, "Job \"SepaTransferSingle\" not supported with this account");

        /* try multi transfer next */
        mj=AH_Job_SepaTransferMulti_new(pro, mu, ma);
        if (!mj) {
          DBG_INFO(AQHBCI_LOGDOMAIN, "Job \"SepaTransferMulti\" not supported with this account");
          return GWEN_ERROR_NOT_AVAILABLE;
        }
      }
    }
    break;

  case AB_Transaction_CommandSepaDebitNote:
    if (!(aFlags & AH_BANK_FLAGS_SEPA_PREFER_SINGLE_DEBITNOTE)) {
      DBG_INFO(AQHBCI_LOGDOMAIN, "Customer prefers multi jobs");

      /* try multi transfer first */
      mj=AH_Job_SepaDebitDatedMultiCreate_new(pro, mu, ma);
      if (!mj) {
        DBG_WARN(AQHBCI_LOGDOMAIN, "SepaDebitDatedMultiCreate not supported with this account");

        /* try single transfer */
        mj=AH_Job_SepaDebitDatedSingleCreate_new(pro, mu, ma);
        if (!mj) {
          DBG_WARN(AQHBCI_LOGDOMAIN,
                   "Job \"SepaDebitDatedSingleCreate\" not supported with this account, trying old single debit");

          /* try old singleDebit job next */
          mj=AH_Job_SepaDebitSingle_new(pro, mu, ma);
          if (!mj) {
            DBG_INFO(AQHBCI_LOGDOMAIN, "Job \"SepaDebitSingle\" not supported with this account");
            return GWEN_ERROR_NOT_AVAILABLE;
          }
        }
      }
    }
    else {
      /* try single job first */
      mj=AH_Job_SepaDebitDatedSingleCreate_new(pro, mu, ma);
      if (!mj) {
        DBG_WARN(AQHBCI_LOGDOMAIN, "SepaDebitDatedSingleCreate not supported with this account");

        /* try old singleDebit job next */
        mj=AH_Job_SepaDebitSingle_new(pro, mu, ma);
        if (!mj) {
          DBG_INFO(AQHBCI_LOGDOMAIN, "Job \"SepaDebitSingle\" not supported with this account");

          /* try multi transfer next */
          mj=AH_Job_SepaDebitDatedMultiCreate_new(pro, mu, ma);
          if (!mj) {
            DBG_INFO(AQHBCI_LOGDOMAIN, "SepaDebitDatedMultiCreate not supported with this account");
            return GWEN_ERROR_NOT_AVAILABLE;
          }
        }
      }
    }
    break;

  case AB_Transaction_CommandSepaFlashDebitNote:
    if (!(aFlags & AH_BANK_FLAGS_SEPA_PREFER_SINGLE_DEBITNOTE)) {
      DBG_INFO(AQHBCI_LOGDOMAIN, "Customer prefers multi jobs");

      /* try multi transfer first */
      mj=AH_Job_SepaCor1DebitDatedMultiCreate_new(pro, mu, ma);
      if (!mj) {
        DBG_WARN(AQHBCI_LOGDOMAIN, "SepaCor1DebitDatedMultiCreate not supported with this account");

        /* try single transfer */
        mj=AH_Job_SepaCor1DebitDatedSingleCreate_new(pro, mu, ma);
        if (!mj) {
          DBG_WARN(AQHBCI_LOGDOMAIN, "Job \"SepaCor1DebitDatedSingleCreate\" not supported with this account");
          return GWEN_ERROR_NOT_AVAILABLE;
        }
      }
    }
    else {
      /* try single job first */
      mj=AH_Job_SepaCor1DebitDatedSingleCreate_new(pro, mu, ma);
      if (!mj) {
        DBG_WARN(AQHBCI_LOGDOMAIN, "SepaCor1DebitDatedSingleCreate not supported with this account");

        /* try multi transfer next */
        mj=AH_Job_SepaCor1DebitDatedMultiCreate_new(pro, mu, ma);
        if (!mj) {
          DBG_INFO(AQHBCI_LOGDOMAIN, "SepaCor1DebitDatedMultiCreate not supported with this account");
          return GWEN_ERROR_NOT_AVAILABLE;
        }
      }
    }
    break;

  case AB_Transaction_CommandSepaCreateStandingOrder:
    mj=AH_Job_SepaStandingOrderCreate_new(pro, mu, ma);
    if (!mj) {
      DBG_INFO(AQHBCI_LOGDOMAIN, "Job not supported with this account");
      return GWEN_ERROR_NOT_AVAILABLE;
    }
    break;
  case AB_Transaction_CommandSepaModifyStandingOrder:
    mj=AH_Job_SepaStandingOrderModify_new(pro, mu, ma);
    if (!mj) {
      DBG_INFO(AQHBCI_LOGDOMAIN, "Job not supported with this account");
      return GWEN_ERROR_NOT_AVAILABLE;
    }
    break;
  case AB_Transaction_CommandSepaDeleteStandingOrder:
    mj=AH_Job_SepaStandingOrderDelete_new(pro, mu, ma);
    if (!mj) {
      DBG_INFO(AQHBCI_LOGDOMAIN, "Job not supported with this account");
      return GWEN_ERROR_NOT_AVAILABLE;
    }
    break;

  case AB_Transaction_CommandSepaGetStandingOrders:
    mj=AH_Job_SepaStandingOrderGet_new(pro, mu, ma);
    if (!mj) {
      DBG_INFO(AQHBCI_LOGDOMAIN, "Job not supported with this account");
      return GWEN_ERROR_NOT_AVAILABLE;
    }
    break;

  case AB_Transaction_CommandGetEStatements:
    mj=AH_Job_GetEStatements_new(pro, mu, ma);
    if (!mj) {
      DBG_INFO(AQHBCI_LOGDOMAIN, "Job not supported with this account");
      return GWEN_ERROR_NOT_AVAILABLE;
    }
    break;

  case AB_Transaction_CommandGetEStatements2:
    mj=AH_Job_GetEStatements2_new(pro, mu, ma);
    if (!mj) {
      DBG_INFO(AQHBCI_LOGDOMAIN, "Job GetEStatements2 not supported with this account");
      return GWEN_ERROR_NOT_AVAILABLE;
    }
    break;

  case AB_Transaction_CommandGetDepot:
    mj=AH_Job_GetDepot_new(pro, mu, ma);
    if (!mj) {
      DBG_INFO(AQHBCI_LOGDOMAIN, "Job not supported with this account");
      return GWEN_ERROR_NOT_AVAILABLE;
    }
    break;

  default:
    DBG_INFO(AQHBCI_LOGDOMAIN,
             "Job not supported by AqHBCI");
    return GWEN_ERROR_NOT_AVAILABLE;
  } /* switch */
  assert(mj);

  DBG_INFO(AQHBCI_LOGDOMAIN, "Job successfully created");
  *pHbciJob=mj;
  return 0;
}



int AH_Provider_GetMultiHbciJob(AB_PROVIDER *pro,
                                AH_OUTBOX *outbox,
                                AB_USER *mu,
                                AB_ACCOUNT *ma,
                                int cmd,
                                AH_JOB **pHbciJob)
{
  AH_JOB *mj=NULL;

  assert(pro);
  assert(mu);

  switch (cmd) {
  case AB_Transaction_CommandSepaTransfer:
    mj=AH_Outbox_FindTransferJob(outbox, mu, ma, "JobSepaTransferMulti");
    break;

  case AB_Transaction_CommandSepaDebitNote:
    mj=AH_Outbox_FindTransferJob(outbox, mu, ma, "JobSepaDebitDatedMultiCreate");
    break;

  default:
    DBG_INFO(AQHBCI_LOGDOMAIN,
             "No Multi jobs defined for this job type");
    break;
  } /* switch */

  if (mj) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "Multi job found");
    *pHbciJob=mj;
    return 0;
  }
  else {
    DBG_INFO(AQHBCI_LOGDOMAIN, "No multi job found");
    return GWEN_ERROR_NOT_FOUND;
  }
}



