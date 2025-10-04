/***************************************************************************
    begin       : Mon Mar 01 2004
    copyright   : (C) 2025 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif


#include "aqhbci/applayer/cbox_psd2.h"

#include "aqhbci/applayer/cbox_itan2.h"
#include "aqhbci/applayer/cbox_vop.h"

#include "aqbanking/i18n_l.h"

#include <gwenhywfar/gui.h>



int AH_OutboxCBox_OpenDialogPsd2_Proc2(AH_OUTBOX_CBOX *cbox, AH_DIALOG *dlg)
{
  AB_PROVIDER *provider;
  AB_USER *user;
  AH_JOB *jDlgOpen;
  int rv;


  DBG_NOTICE(AQHBCI_LOGDOMAIN, "Creating dialog open request");

  provider=AH_OutboxCBox_GetProvider(cbox);
  user=AH_OutboxCBox_GetUser(cbox);

    /* dialog open job */
  jDlgOpen=AH_Job_new("JobDialogInit", provider, user, 0, 0);
  if (!jDlgOpen) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "Could not create job JobDialogInit");
    return GWEN_ERROR_GENERIC;
  }
  AH_Job_SetCode(jDlgOpen, "HKIDN"); /* needed for HKTAN6 in SCA mode */
  AH_Job_AddSigner(jDlgOpen, AB_User_GetUserId(user));

  /* need signature in any case */
  AH_Job_AddFlags(jDlgOpen, AH_JOB_FLAGS_SIGN);

  rv=AH_OutboxCBox_OpenDialogPsd2WithJob_Proc2(cbox, dlg, jDlgOpen);
  if (rv) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
    AH_Job_free(jDlgOpen);
    return rv;
  }

  AH_Job_free(jDlgOpen);
  return rv;
}



int AH_OutboxCBox_OpenDialogPsd2WithJob_Proc2(AH_OUTBOX_CBOX *cbox, AH_DIALOG *dlg, AH_JOB *jDlgOpen)
{
  AB_USER *user;
  int rv;

  DBG_NOTICE(AQHBCI_LOGDOMAIN, "Creating dialog open request");

  user=AH_OutboxCBox_GetUser(cbox);

  AH_Dialog_SetItanProcessType(dlg, 0);

  /* use strong authentication */
  AH_Dialog_AddFlags(dlg, AH_DIALOG_FLAGS_SCA);

  if (AH_User_HasTanMethodOtherThan(user, 999)) {
    /* only use itan if any other mode than singleStep is available
     * and the job queue does not request non-ITAN mode
     */
    rv=AH_OutboxCBox_SelectItanMode(cbox, dlg);
    if (rv) {
      DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
      return rv;
    }
  }
  else {
    DBG_INFO(AQHBCI_LOGDOMAIN, "We have no list of allowed two-step TAN methods, maybe you should request TAN methods.");
    GWEN_Gui_ProgressLog(0,
                         GWEN_LoggerLevel_Warning,
                         I18N("We have no list of allowed two-step TAN methods, maybe you should request TAN methods."));
    AH_Dialog_SetItanMethod(dlg, 999);
    AH_Dialog_SetItanProcessType(dlg, 1);
    AH_Dialog_SetTanJobVersion(dlg, 0);
  }

  rv=AH_OutboxCBox_SendAndReceiveJobWithTanAndVpp(cbox, dlg, jDlgOpen);
  if (rv) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }

  DBG_NOTICE(AQHBCI_LOGDOMAIN, "Dialog open request done.");
  rv=AH_Job_CommitSystemData(jDlgOpen, 0);
  if (rv) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }
  return rv;
}




