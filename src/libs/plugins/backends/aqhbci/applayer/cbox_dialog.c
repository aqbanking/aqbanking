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


#include "aqhbci/applayer/cbox_dialog.h"

#include "aqhbci/applayer/cbox_psd2.h"
#include "aqhbci/applayer/cbox_hbci.h"
#include "aqhbci/applayer/cbox_queue.h"
#include "aqhbci/admjobs/jobtan_l.h"

#include "aqbanking/i18n_l.h"

#include <gwenhywfar/gui.h>



int AH_OutboxCBox_OpenDialog(AH_OUTBOX_CBOX *cbox,
                             AH_DIALOG *dlg,
                             uint32_t jFlags)
{
  AB_PROVIDER *provider;
  AB_USER *user;
  int rv;

  assert(cbox);
  assert(dlg);

  provider=AH_OutboxCBox_GetProvider(cbox);
  user=AH_OutboxCBox_GetUser(cbox);

  if (AH_User_GetCryptMode(user)==AH_CryptMode_Pintan) {
    int selectedTanVersion;

    selectedTanVersion=AH_User_GetSelectedTanMethod(user)/1000;

    DBG_INFO(AQHBCI_LOGDOMAIN, "CryptMode is PINTAN");
    if (selectedTanVersion>=6) {
      AH_JOB *jTan;

      DBG_INFO(AQHBCI_LOGDOMAIN, "User-selected TAN job version is 6 or newer (%d)", selectedTanVersion);

      /* check for PSD2: HKTAN version >= 6 available? if so -> use that */
      jTan=AH_Job_Tan_new(provider, user, 4, selectedTanVersion);
      if (jTan) {
        AH_Job_free(jTan);
        DBG_INFO(AQHBCI_LOGDOMAIN, "TAN job version is available");
        DBG_NOTICE(AQHBCI_LOGDOMAIN, "Using PSD2 code to init dialog");
        rv=AH_OutboxCBox_OpenDialogPsd2_Proc2(cbox, dlg);
        if (rv!=0) {
          DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
          return rv;
        }
        return rv;
      }
      else {
        DBG_NOTICE(AQHBCI_LOGDOMAIN, "Not using PSD2 code: HKTAN version %d not supported by the bank", selectedTanVersion);
      }
    }
    else {
      DBG_NOTICE(AQHBCI_LOGDOMAIN, "Not using PSD2 code: User selected HKTAN version lesser than 6.");
    }
  }

  /* fall back */
  DBG_NOTICE(AQHBCI_LOGDOMAIN, "Using standard HBCI code to init dialog");
  rv=AH_OutboxCBox_OpenDialog_Hbci(cbox, dlg, jFlags);
  if (rv!=0) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }
  return rv;
}



int AH_OutboxCBox_OpenDialogWithJob(AH_OUTBOX_CBOX *cbox, AH_DIALOG *dlg, AH_JOB *jDlg)
{
  AB_PROVIDER *provider;
  AB_USER *user;
  int rv;

  assert(cbox);
  assert(dlg);

  provider=AH_OutboxCBox_GetProvider(cbox);
  user=AH_OutboxCBox_GetUser(cbox);

  if (AH_User_GetCryptMode(user)==AH_CryptMode_Pintan) {
    int selectedTanVersion;

    selectedTanVersion=AH_User_GetSelectedTanMethod(user)/1000;

    DBG_INFO(AQHBCI_LOGDOMAIN, "CryptMode is PINTAN");
    if (selectedTanVersion>=6) {
      AH_JOB *jTan;

      DBG_INFO(AQHBCI_LOGDOMAIN, "User-selected TAN job version is 6 or newer (%d)", selectedTanVersion);

      /* check for PSD2: HKTAN version >= 6 available? if so -> use that */
      jTan=AH_Job_Tan_new(provider, user, 4, selectedTanVersion);
      if (jTan) {
        AH_Job_free(jTan);
        DBG_INFO(AQHBCI_LOGDOMAIN, "TAN job version is available");
        DBG_NOTICE(AQHBCI_LOGDOMAIN, "Using PSD2 code to init dialog");
        rv=AH_OutboxCBox_OpenDialogPsd2WithJob_Proc2(cbox, dlg, jDlg);
        if (rv!=0) {
          DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
          return rv;
        }
        return rv;
      }
      else {
        DBG_NOTICE(AQHBCI_LOGDOMAIN, "Not using PSD2 code: HKTAN version %d not supported by the bank", selectedTanVersion);
      }
    }
    else {
      DBG_NOTICE(AQHBCI_LOGDOMAIN, "Not using PSD2 code: User selected HKTAN version lesser than 6.");
    }
  }

  /* fall back */
  DBG_NOTICE(AQHBCI_LOGDOMAIN, "Using standard HBCI code to init dialog");
  rv=AH_OutboxCBox_OpenDialogWithJob_Hbci(cbox, dlg, jDlg);
  if (rv!=0) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }
  return rv;
}




int AH_OutboxCBox_CloseDialog(AH_OUTBOX_CBOX *cbox, AH_DIALOG *dlg, uint32_t jFlags)
{
  AB_PROVIDER *provider;
  AB_USER *user;
  AH_JOB *jDlgClose;
  GWEN_DB_NODE *db;
  uint32_t dlgFlags;
  int rv;

  provider=AH_OutboxCBox_GetProvider(cbox);
  user=AH_OutboxCBox_GetUser(cbox);

  GWEN_Gui_ProgressLog(0, GWEN_LoggerLevel_Notice, I18N("Closing dialog"));
  DBG_NOTICE(AQHBCI_LOGDOMAIN, "Sending dialog close request (flags=%08x)", jFlags);
  dlgFlags=AH_Dialog_GetFlags(dlg);
  jDlgClose=AH_Job_new("JobDialogEnd", provider, user, 0, 0);
  if (!jDlgClose) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "Could not create job JobDialogEnd");
    return GWEN_ERROR_GENERIC;
  }

  /* set some parameters */
  db=AH_Job_GetArguments(jDlgClose);
  assert(db);
  GWEN_DB_SetCharValue(db,
                       GWEN_DB_FLAGS_DEFAULT | GWEN_DB_FLAGS_OVERWRITE_VARS,
                       "dialogId",
                       AH_Dialog_GetDialogId(dlg));

  /* handle signing and encryption */
  if (dlgFlags & AH_DIALOG_FLAGS_ANONYMOUS) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "Will not encrypt and sign dialog close request");
    AH_Job_SubFlags(jDlgClose, AH_JOB_FLAGS_CRYPT | AH_JOB_FLAGS_SIGN | AH_JOB_FLAGS_NEEDTAN);
    AH_Job_AddFlags(jDlgClose, AH_JOB_FLAGS_NOITAN);
  }
  else {
    /* possibly sign job */
    if (jFlags & AH_JOB_FLAGS_SIGN) {
      DBG_INFO(AQHBCI_LOGDOMAIN, "Will sign dialog close request");
      AH_Job_AddSigner(jDlgClose, AB_User_GetUserId(user));
      AH_Job_AddFlags(jDlgClose, AH_JOB_FLAGS_SIGN | AH_JOB_FLAGS_NEEDSIGN);
    }
    else {
      DBG_INFO(AQHBCI_LOGDOMAIN, "Will not sign dialog close request");
      AH_Job_SubFlags(jDlgClose, AH_JOB_FLAGS_SIGN | AH_JOB_FLAGS_NEEDSIGN);
    }

    /* possibly encrypt job */
    if (jFlags & AH_JOB_FLAGS_CRYPT) {
      DBG_INFO(AQHBCI_LOGDOMAIN, "Will encrypt dialog close request");
      AH_Job_AddFlags(jDlgClose, AH_JOB_FLAGS_CRYPT | AH_JOB_FLAGS_NEEDCRYPT);
    }
    else {
      DBG_INFO(AQHBCI_LOGDOMAIN, "Will not encrypt dialog close request");
      AH_Job_SubFlags(jDlgClose, AH_JOB_FLAGS_CRYPT | AH_JOB_FLAGS_NEEDCRYPT);
    }

    /* possibly set NOITAN job */
    if (jFlags & AH_JOB_FLAGS_NOITAN) {
      DBG_INFO(AQHBCI_LOGDOMAIN, "Disable ITAN mode for dialog close request");
      AH_Job_AddFlags(jDlgClose, AH_JOB_FLAGS_NOITAN);
    }
    else {
      DBG_INFO(AQHBCI_LOGDOMAIN, "Will not disable ITAN mode for dialog close request");
      AH_Job_SubFlags(jDlgClose, AH_JOB_FLAGS_NOITAN);
    }
  }

  rv=AH_OutboxCBox_SendAndReceiveJob(cbox, dlg, jDlgClose);
  if (rv) {
    DBG_NOTICE(AQHBCI_LOGDOMAIN, "Could not exchange message");
    AH_Job_free(jDlgClose);
    return rv;
  }
  DBG_NOTICE(AQHBCI_LOGDOMAIN, "Dialog closed");
  rv=AH_Job_CommitSystemData(jDlgClose, 0);
  AH_Job_free(jDlgClose);
  if (rv) {
    DBG_NOTICE(AQHBCI_LOGDOMAIN, "Could not commit system data");
    return rv;
  }
  return 0;
}







