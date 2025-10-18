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

#include "aqbanking/types/transaction.h"
#include "aqbanking/i18n_l.h"

#include <gwenhywfar/gui.h>



static AB_TRANSACTION_VOPRESULT _vopResultCodeToTransactionVopResult(int i);




int AH_OutboxCBox_LetUserConfirmVopResult(AH_OUTBOX_CBOX *cbox, AH_JOB *workJob, AH_JOB *vppJob, const char *sMsg)
{
  AB_PROVIDER *provider;
  AB_BANKING *ab;
  AB_USER *user;
  GWEN_BUFFER *guiBuf;
  const char *sUserName;
  const char *sBankName=NULL;
  AB_BANKINFO *bankInfo;
  int rv;

  provider=AH_OutboxCBox_GetProvider(cbox);
  ab=AB_Provider_GetBanking(provider);
  user=AH_OutboxCBox_GetUser(cbox);
  sUserName=AB_User_GetUserId(user);

  /* find bank name */
  bankInfo=AB_Banking_GetBankInfo(ab, "de", "*", AB_User_GetBankCode(user));
  if (bankInfo)
    sBankName=AB_BankInfo_GetBankName(bankInfo);
  if (!sBankName)
    sBankName=AB_User_GetBankCode(user);

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
                         AH_Job_GetCode(workJob));

  AB_BankInfo_free(bankInfo);

  rv=GWEN_Gui_MessageBox(GWEN_GUI_MSG_FLAGS_TYPE_INFO | GWEN_GUI_MSG_FLAGS_CONFIRM_B1 | GWEN_GUI_MSG_FLAGS_SEVERITY_DANGEROUS,
                         I18N("Verification of Payee Result"),
                         GWEN_Buffer_GetStart(guiBuf),
                         I18N("Approve"),
                         I18N("Abort"),
                         NULL,
                         0);
  GWEN_Buffer_free(guiBuf);
  if (rv!=1) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "Not confirming payee(s) (%d)", rv);
    return GWEN_ERROR_USER_ABORTED;
  }

  return 0;
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
  
        sRemoteName=AB_Transaction_GetRemoteName(t);
        sRemoteIban=AB_Transaction_GetRemoteIban(t);
        if (sRemoteIban && *sRemoteIban) {
          const AH_VOP_RESULT *vr;
  
          vr=AH_VopResult_List_GetByIbanAndName(vrList, sRemoteIban, sRemoteName);
          if (vr) {
            const char *sAltName;
            int resultCode;
  
            sAltName=AH_VopResult_GetAltRemoteName(vr);
            resultCode=AH_VopResult_GetResult(vr);
            if (sAltName) {
              DBG_ERROR(AQHBCI_LOGDOMAIN,
                        "Result for transfer: %s: \"%s\" -> \"%s\" (%s)",
                        sRemoteIban, sRemoteName?sRemoteName:"<no name>", sAltName?sAltName:"<no name>",
                        AH_VopResultCode_toString(resultCode));
            }
            else {
              DBG_ERROR(AQHBCI_LOGDOMAIN, "Result for transfer: %s (%s)", sRemoteIban, AH_VopResultCode_toString(resultCode));
            }
            AB_Transaction_SetVopResult(t, _vopResultCodeToTransactionVopResult(resultCode));
            AB_Transaction_SetUltimateCreditor(t, sAltName);
          }
          else {
            DBG_ERROR(AQHBCI_LOGDOMAIN, "No result found for transfer, assuming okay");
            AB_Transaction_SetVopResult(t, AB_Transaction_VopResultNone);
          }
        }
  
        t=AB_Transaction_List_Next(t);
      }
    }
  }
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






