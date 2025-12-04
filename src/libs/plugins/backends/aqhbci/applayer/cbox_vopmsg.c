/***************************************************************************
    begin       : Fri Oct 3 2025
    copyright   : (C) 2025 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif


#include "aqhbci/applayer/cbox_vopmsg.h"
#include "aqhbci/dialogs/dlg_vop.h"
#include "aqhbci/admjobs/jobvpp.h"

#include "aqbanking/types/transaction.h"
#include "aqbanking/i18n_l.h"

#include <gwenhywfar/gui.h>



static void _applyVopResultToTransaction(const AH_VOP_RESULT *vr, const char *sRemoteIban, const char *sRemoteName, AB_TRANSACTION *t);
static AB_TRANSACTION_VOPRESULT _vopResultCodeToTransactionVopResult(int i);
static int _showSimpleGuiMessage(const char *sJobName, const char *sBankName, const char *sUserName, const char *sMsg);




int AH_OutboxCBox_LetUserConfirmVopResult(AH_OUTBOX_CBOX *cbox, AH_JOB *workJob, AH_JOB *vppJob, const char *sMsg)
{
  AB_PROVIDER *provider;
  AB_BANKING *ab;
  AB_USER *user;
  const char *sUserName;
  const char *sBankName;
  const char *sJobName;
  AB_BANKINFO *bankInfo;
  const AH_VOP_RESULT_LIST *resultList;
  GWEN_DIALOG *dlg;
  int rv;

  provider=AH_OutboxCBox_GetProvider(cbox);
  ab=AB_Provider_GetBanking(provider);
  user=AH_OutboxCBox_GetUser(cbox);
  sUserName=AB_User_GetUserId(user);
  sJobName=AH_Job_GetName(workJob);

  resultList=vppJob?AH_Job_VPP_GetResultList(vppJob):NULL;
  if (!(sMsg && *sMsg) && (resultList==NULL || (resultList && AH_VopResult_List_HasOnlyMatches(resultList)))) {
    DBG_WARN(AQHBCI_LOGDOMAIN, "No msg, no non-matching results, silently accepting.");
    return 0;
  }

  /* find bank name */
  bankInfo=AB_Banking_GetBankInfo(ab, "de", "*", AB_User_GetBankCode(user));
  sBankName=bankInfo?AB_BankInfo_GetBankName(bankInfo):NULL;
  if (!sBankName)
    sBankName=AB_User_GetBankCode(user);

  dlg=AH_VopDialog_new(sJobName, sBankName, sUserName, sMsg, resultList);
  if (dlg) {
    rv=GWEN_Gui_ExecDialog(dlg, 0);
    GWEN_Dialog_free(dlg);
    if (rv<0) {
      if (rv!=GWEN_ERROR_NOT_IMPLEMENTED) {
        DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
        AB_BankInfo_free(bankInfo);
        return rv;
      }
      /* fall-through */
    }
    else if (rv==0) {
      /* rejected */
      DBG_INFO(AQHBCI_LOGDOMAIN, "Rejected");
      AB_BankInfo_free(bankInfo);
      return GWEN_ERROR_USER_ABORTED;
    }
    else {
      DBG_INFO(AQHBCI_LOGDOMAIN, "Accepted");
      AB_BankInfo_free(bankInfo);
      return 0;
    }
  }

  DBG_ERROR(AQHBCI_LOGDOMAIN, "Error creating or running dialog, trying simple dialog");
  rv=_showSimpleGuiMessage(sJobName, sBankName, sUserName, sMsg);
  AB_BankInfo_free(bankInfo);
  return rv;
}



void AH_OutboxCBox_ApplyVopResultsToTransfers(AH_JOB *workJob, const AH_VOP_RESULT_LIST *vrList)
{
  if (vrList) {
    AB_TRANSACTION_LIST *transferList;
  
    transferList=AH_Job_GetTransferList(workJob);
    if (transferList) {
      AB_TRANSACTION *t;

      t=AB_Transaction_List_First(transferList);
      while(t) {
	const char *sRemoteIban;
	const char *sRemoteName;
	const AH_VOP_RESULT *vr;

	sRemoteName=AB_Transaction_GetRemoteName(t);
	sRemoteIban=AB_Transaction_GetRemoteIban(t);
	if (AB_Transaction_List_GetCount(transferList)==1) {
	  /* single transfer in job, so the result MUST be for that */
	  vr=AH_VopResult_List_First(vrList);
	  _applyVopResultToTransaction(vr, sRemoteIban, sRemoteName, t);
	}
	else if (sRemoteIban && *sRemoteIban) {
	  vr=AH_VopResult_List_GetByIbanAndName(vrList, sRemoteIban, sRemoteName);
	  if (vr)
	    _applyVopResultToTransaction(vr, sRemoteIban, sRemoteName, t);
	  else {
            DBG_INFO(AQHBCI_LOGDOMAIN, "No result found for transfer involving %s, assuming okay", sRemoteIban);
	    AB_Transaction_SetVopResult(t, AB_Transaction_VopResultNone);
	  }
	}

	t=AB_Transaction_List_Next(t);
      }
    }
  }
}



void _applyVopResultToTransaction(const AH_VOP_RESULT *vr, const char *sRemoteIban, const char *sRemoteName, AB_TRANSACTION *t)
{
  const char *sAltName;
  int resultCode;
  
  sAltName=AH_VopResult_GetAltRemoteName(vr);
  resultCode=AH_VopResult_GetResult(vr);
  if (sAltName) {
    DBG_INFO(AQHBCI_LOGDOMAIN,
	     "Result for transfer: %s: \"%s\" -> \"%s\" (%s)",
	     sRemoteIban?sRemoteIban:"<no iban>", sRemoteName?sRemoteName:"<no name>", sAltName?sAltName:"<no name>",
	     AH_VopResultCode_toString(resultCode));
  }
  else {
    DBG_INFO(AQHBCI_LOGDOMAIN,
	     "Result for transfer: %s (%s)",
	     sRemoteIban?sRemoteIban:"<no iban>",
	     AH_VopResultCode_toString(resultCode));
  }
  AB_Transaction_SetVopResult(t, _vopResultCodeToTransactionVopResult(resultCode));
  AB_Transaction_SetUltimateCreditor(t, sAltName);
}



AB_TRANSACTION_VOPRESULT _vopResultCodeToTransactionVopResult(int i)
{
  switch(i) {
  case AH_VopResultCodeNone:         return AB_Transaction_VopResultNone;
  case AH_VopResultCodeMatch:        return AB_Transaction_VopResultMatch;
  case AH_VopResultCodeCloseMatch:   return AB_Transaction_VopResultCloseMatch;
  case AH_VopResultCodeNoMatch:      return AB_Transaction_VopResultNoMatch;
  case AH_VopResultCodeNotAvailable: return AB_Transaction_VopResultNotAvailable;
  case AH_VopResultCodePending:      return AB_Transaction_VopResultPending;
  default:                           return AB_Transaction_VopResultNone;
  }
}



int _showSimpleGuiMessage(const char *sJobName, const char *sBankName, const char *sUserName, const char *sMsg)
{
  GWEN_BUFFER *guiBuf;
  int rv;

  guiBuf=GWEN_Buffer_new(0, 256, 0, 1);
  GWEN_Buffer_AppendArgs(guiBuf,
                         I18N("Result of Verification of Payee process at the bank (user %s at %s):\n"
                              "%s\n"
                              "\n"
                              "If you still want to execute the job \"%s\" click \"Approve\".\n"
                              "\n"
                              "Please note that in that case the risk for executing the given job will move from the bank\n"
                              "to you. Always make sure you have the correct payee in your transfers!"),
                         sUserName?sUserName:"<no user id>",
                         sBankName?sBankName:"<no bank name>",
                         sMsg?sMsg:I18N("<no msg from bank>"),
                         sJobName);

  rv=GWEN_Gui_MessageBox(GWEN_GUI_MSG_FLAGS_TYPE_INFO | GWEN_GUI_MSG_FLAGS_CONFIRM_B1 | GWEN_GUI_MSG_FLAGS_SEVERITY_DANGEROUS,
                         I18N("Verification of Payee Result"),
                         GWEN_Buffer_GetStart(guiBuf),
                         I18N("Approve"),
                         I18N("Abort"),
                         NULL,
                         0);
  GWEN_Buffer_free(guiBuf);
  if (rv!=1) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "Not confirming payee(s) (%d)", rv);
    return GWEN_ERROR_USER_ABORTED;
  }

  DBG_INFO(AQHBCI_LOGDOMAIN, "User accepted.");
  return 0;
}





