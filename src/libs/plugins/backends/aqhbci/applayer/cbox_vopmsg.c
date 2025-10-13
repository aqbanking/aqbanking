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

#include "aqbanking/i18n_l.h"

#include <gwenhywfar/gui.h>



int AH_OutboxCBox_LetUserConfirmVopResult(AH_OUTBOX_CBOX *cbox, AH_JOB *workJob, const char *sMsg)
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
                         sMsg?sMsg:"<no msg from bank>",
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



