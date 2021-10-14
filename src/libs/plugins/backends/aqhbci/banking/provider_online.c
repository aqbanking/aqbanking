/***************************************************************************
    begin       : Tue Jun 03 2018
    copyright   : (C) 2018 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "provider_online.h"

#include "aqhbci/banking/provider_l.h"
#include "aqhbci/applayer/adminjobs_l.h"
#include "aqhbci/msglayer/dialog_l.h"

#include "aqhbci/admjobs/jobgetkeys_l.h"
#include "aqhbci/admjobs/jobsendkeys_l.h"
#include "aqhbci/admjobs/jobchangekeys_l.h"
#include "aqhbci/admjobs/jobgetsepainfo_l.h"
#include "aqhbci/admjobs/jobgetsysid_l.h"
#include "aqhbci/admjobs/jobgetbankinfo_l.h"
#include "aqhbci/admjobs/jobunblockpin_l.h"
#include "aqhbci/admjobs/jobgettargetacc_l.h"

#include <gwenhywfar/gui.h>




int AH_Provider_GetAccounts(AB_PROVIDER *pro, AB_USER *u,
                            AB_IMEXPORTER_CONTEXT *ctx,
                            int withProgress, int nounmount, int doLock)
{
  AB_BANKING *ab;
  AH_HBCI *h;
  AH_JOB *job;
  AH_OUTBOX *ob;
  int rv;

  assert(pro);
  assert(u);

  ab=AB_Provider_GetBanking(pro);
  assert(ab);

  h=AH_Provider_GetHbci(pro);
  assert(h);

  job=AH_Job_UpdateBank_new(pro, u);
  if (!job) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "Job not supported, should not happen");
    return GWEN_ERROR_GENERIC;
  }
  AH_Job_AddSigner(job, AB_User_GetUserId(u));

  ob=AH_Outbox_new(pro);
  AH_Outbox_AddJob(ob, job);

  rv=AH_Outbox_Execute(ob, ctx, withProgress, 1, doLock);
  AH_Outbox_free(ob);
  if (rv) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "Could not execute outbox.\n");
    AH_Job_free(job);
    if (!nounmount)
      AB_Banking_ClearCryptTokenList(AH_HBCI_GetBankingApi(h));
    return rv;
  }

  /* always try to commit, even when there are errors */
  rv=AH_Job_Commit(job, doLock);
  if (rv) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "Could not commit result.\n");
    AH_Job_free(job);
    if (!nounmount)
      AB_Banking_ClearCryptTokenList(AH_HBCI_GetBankingApi(h));
    return rv;
  }

  if (AH_Job_HasErrors(job)) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "Job has errors, but accounts may have been received.");
    AH_Job_free(job);
    if (!nounmount)
      AB_Banking_ClearCryptTokenList(AH_HBCI_GetBankingApi(h));
    return GWEN_ERROR_GENERIC;
  }

  AH_Job_free(job);
  if (!nounmount)
    AB_Banking_ClearCryptTokenList(AH_HBCI_GetBankingApi(h));
  return 0;
}



int AH_Provider_GetBankInfo(AB_PROVIDER *pro, AB_USER *u,
                            AB_IMEXPORTER_CONTEXT *ctx,
                            int withTanSeg,
                            int withProgress, int nounmount, int doLock)
{
  AB_BANKING *ab;
  AH_HBCI *h;
  AH_JOB *job;
  AH_OUTBOX *ob;
  int rv;

  assert(pro);
  assert(u);

  ab=AB_Provider_GetBanking(pro);
  assert(ab);

  h=AH_Provider_GetHbci(pro);
  assert(h);

  job=AH_Job_GetBankInfo_new(pro, u, withTanSeg);
  if (!job) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "Job not supported, should not happen");
    return GWEN_ERROR_GENERIC;
  }

  ob=AH_Outbox_new(pro);
  AH_Outbox_AddJob(ob, job);

  rv=AH_Outbox_Execute(ob, ctx, withProgress, 1, doLock);
  AH_Outbox_free(ob);
  if (rv) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "Could not execute outbox.\n");
    AH_Job_free(job);
    if (!nounmount)
      AB_Banking_ClearCryptTokenList(AH_HBCI_GetBankingApi(h));
    return rv;
  }

  if (AH_Job_HasErrors(job)) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "Job has errors");
    // TODO: show errors
    AH_Job_free(job);
    if (!nounmount)
      AB_Banking_ClearCryptTokenList(AH_HBCI_GetBankingApi(h));
    return GWEN_ERROR_GENERIC;
  }
  else {
    rv=AH_Job_Commit(job, doLock);
    if (rv) {
      DBG_ERROR(AQHBCI_LOGDOMAIN, "Could not commit result.\n");
      AH_Job_free(job);
      if (!nounmount)
        AB_Banking_ClearCryptTokenList(AH_HBCI_GetBankingApi(h));
      return rv;
    }
  }

  AH_Job_free(job);
  if (!nounmount)
    AB_Banking_ClearCryptTokenList(AH_HBCI_GetBankingApi(h));
  return 0;
}



int AH_Provider_GetSysId(AB_PROVIDER *pro, AB_USER *u,
                         AB_IMEXPORTER_CONTEXT *ctx,
                         int withProgress, int nounmount, int doLock)
{
  AB_BANKING *ab;
  AH_HBCI *h;
  AH_JOB *job;
  int rv;
  const char *s;
  int i;
  char tbuf[256];

  assert(pro);
  assert(u);

  ab=AB_Provider_GetBanking(pro);
  assert(ab);

  h=AH_Provider_GetHbci(pro);
  assert(h);

  job=0;
  rv=0;
  for (i=0; ; i++) {
    AH_OUTBOX *ob;

    job=AH_Job_GetSysId_new(pro, u);
    if (!job) {
      DBG_ERROR(AQHBCI_LOGDOMAIN, "Job not supported, should not happen");
      return GWEN_ERROR_GENERIC;
    }
    AH_Job_AddSigner(job, AB_User_GetUserId(u));

    ob=AH_Outbox_new(pro);
    AH_Outbox_AddJob(ob, job);

    rv=AH_Outbox_Execute(ob, ctx, withProgress, 1, doLock);
    AH_Outbox_free(ob);
    if (rv) {
      DBG_ERROR(AQHBCI_LOGDOMAIN, "Could not execute outbox.\n");
      AH_Job_free(job);
      if (!nounmount)
        AB_Banking_ClearCryptTokenList(AH_HBCI_GetBankingApi(h));
      return rv;
    }

    /* check whether we received a sysid */
    s=AH_Job_GetSysId_GetSysId(job);
    if (s && *s) {
      /* we did, commit the job and break loop */
      rv=AH_Job_Commit(job, doLock);
      if (rv<0) {
        DBG_ERROR(AQHBCI_LOGDOMAIN, "Could not commit result.\n");
        AH_Job_free(job);
        if (!nounmount)
          AB_Banking_ClearCryptTokenList(AH_HBCI_GetBankingApi(h));
        return rv;
      }
      break;
    }

    if (AH_Job_HasItanResult(job)) {
      GWEN_Gui_ProgressLog(0,
                           GWEN_LoggerLevel_Notice,
                           I18N("Adjusting to iTAN modes of the server"));
      rv=AH_Job_Commit(job, doLock);
      if (rv) {
        DBG_ERROR(AQHBCI_LOGDOMAIN, "Could not commit result.\n");
        AH_Job_free(job);
        if (!nounmount)
          AB_Banking_ClearCryptTokenList(AH_HBCI_GetBankingApi(h));
        return rv;
      }

#if 0
      /* save user in order to get the info written to config database for
       * inspection while debugging
       */
      rv=AB_Banking_SaveUser(ab, u);
      if (rv<0) {
        DBG_ERROR(AQHBCI_LOGDOMAIN, "Error saving user (%d)", rv);
        AH_Outbox_free(ob);
        if (!nounmount)
          AB_Banking_ClearCryptTokenList(AH_HBCI_GetBankingApi(h));
        return rv;
      }
#endif

      rv=GWEN_Gui_ProgressLog(0,
                              GWEN_LoggerLevel_Notice,
                              I18N("Retrying to get system id."));
      if (rv) {
        DBG_ERROR(AQHBCI_LOGDOMAIN, "Error in progress log, maybe user aborted?");
        AH_Job_free(job);
        if (!nounmount)
          AB_Banking_ClearCryptTokenList(AH_HBCI_GetBankingApi(h));
        return rv;
      }
    }
    else {
      DBG_ERROR(AQHBCI_LOGDOMAIN, "Job has no system id and no iTAN results");
      // TODO: show errors
      AH_Job_free(job);
      if (!nounmount)
        AB_Banking_ClearCryptTokenList(AH_HBCI_GetBankingApi(h));
      return GWEN_ERROR_GENERIC;
    }

    AH_Job_free(job);
    if (i>1) {
      DBG_ERROR(AQHBCI_LOGDOMAIN, "Tried too often, giving up");
      GWEN_Gui_ProgressLog(0,
                           GWEN_LoggerLevel_Error,
                           I18N("Could not get system id after multiple trials"));
      if (!nounmount)
        AB_Banking_ClearCryptTokenList(AH_HBCI_GetBankingApi(h));
      return GWEN_ERROR_GENERIC;
    }
  } /* for */

  /* lock user */
  if (doLock) {
    rv=AB_Provider_BeginExclUseUser(pro, u);
    if (rv) {
      DBG_ERROR(AQHBCI_LOGDOMAIN, "Could not lock user (%d)\n", rv);
      AH_Job_free(job);
      if (!nounmount)
        AB_Banking_ClearCryptTokenList(AH_HBCI_GetBankingApi(h));
      return rv;
    }
  }

  s=AH_Job_GetSysId_GetSysId(job);
  if (!s) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "No system id");
    if (doLock)
      AB_Provider_EndExclUseUser(pro, u, 1);
    AH_Job_free(job);
    if (!nounmount)
      AB_Banking_ClearCryptTokenList(AH_HBCI_GetBankingApi(h));
    return GWEN_ERROR_NO_DATA;
  }

  AH_User_SetSystemId(u, s);
  AH_Job_free(job);

  /* unlock user */
  if (doLock) {
    rv=AB_Provider_EndExclUseUser(pro, u, 0);
    if (rv<0) {
      DBG_INFO(AQHBCI_LOGDOMAIN,
               "Could not unlock customer [%s] (%d)",
               AB_User_GetCustomerId(u), rv);
      snprintf(tbuf, sizeof(tbuf)-1,
               I18N("Could not unlock user %s (%d)"),
               AB_User_GetUserId(u), rv);
      tbuf[sizeof(tbuf)-1]=0;
      GWEN_Gui_ProgressLog(0,
                           GWEN_LoggerLevel_Error,
                           tbuf);
      AB_Provider_EndExclUseUser(pro, u, 1);
      if (!nounmount)
        AB_Banking_ClearCryptTokenList(AH_HBCI_GetBankingApi(h));
      return rv;
    }
  }

  if (!nounmount)
    AB_Banking_ClearCryptTokenList(AH_HBCI_GetBankingApi(h));

  return 0;
}



int AH_Provider_GetServerKeys(AB_PROVIDER *pro, AB_USER *u,
                              AB_IMEXPORTER_CONTEXT *ctx,
                              int withProgress, int nounmount, int doLock)
{
  AB_BANKING *ab;
  AH_HBCI *h;
  AH_JOB *job;
  AH_OUTBOX *ob;
  int rv;
  GWEN_CRYPT_TOKEN *ct;
  const GWEN_CRYPT_TOKEN_CONTEXT *cctx;
  const char *s;

  assert(pro);
  assert(u);

  ab=AB_Provider_GetBanking(pro);
  assert(ab);

  h=AH_Provider_GetHbci(pro);
  assert(h);

  job=AH_Job_GetKeys_new(pro, u);
  if (!job) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "Job not supported, should not happen");
    return GWEN_ERROR_GENERIC;
  }

  ob=AH_Outbox_new(pro);
  AH_Outbox_AddJob(ob, job);

  rv=AH_Outbox_Execute(ob, ctx, withProgress, 1, doLock);

  AH_Outbox_free(ob);
  if (rv) {
    GWEN_Gui_ProgressLog(0,
                         GWEN_LoggerLevel_Error,
                         I18N("Could not execute outbox."));
    AH_Job_free(job);
    if (!nounmount)
      AB_Banking_ClearCryptTokenList(AH_HBCI_GetBankingApi(h));
    return rv;
  }

  /* Store crypt and sign keys in database */
  if (AH_Job_GetKeys_GetCryptKeyInfo(job)==NULL && AH_Job_GetKeys_GetSignKeyInfo(job)==NULL) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "No crypt key and no sign key received");
    GWEN_Gui_ProgressLog(0,
                         GWEN_LoggerLevel_Error,
                         I18N("No crypt key and no sign key received."));
    AH_Job_free(job);
    if (!nounmount)
      AB_Banking_ClearCryptTokenList(AH_HBCI_GetBankingApi(h));
    return GWEN_ERROR_GENERIC;
  }
  else {
    rv=AH_Job_Commit(job, doLock);
    if (rv) {
      DBG_ERROR(AQHBCI_LOGDOMAIN, "Could not commit result.\n");
      GWEN_Gui_ProgressLog(0,
                           GWEN_LoggerLevel_Error,
                           I18N("Could not commit result"));
      AH_Job_free(job);
      if (!nounmount)
        AB_Banking_ClearCryptTokenList(AH_HBCI_GetBankingApi(h));
      return rv;
    }
  }

  /* lock user */
  if (doLock) {
    rv=AB_Provider_BeginExclUseUser(pro, u);
    if (rv) {
      DBG_ERROR(AQHBCI_LOGDOMAIN, "Could not lock user (%d)\n", rv);
      AH_Job_free(job);
      if (!nounmount)
        AB_Banking_ClearCryptTokenList(AH_HBCI_GetBankingApi(h));
      return rv;
    }
  }

  s=AH_User_GetPeerId(u);
  if (!s || !*s) {
    s=AH_Job_GetKeys_GetPeerId(job);
    if (s && *s) {
      char tbuf[256];

      snprintf(tbuf, sizeof(tbuf)-1, I18N("Setting peer ID to \"%s\")"), s);
      tbuf[sizeof(tbuf)-1]=0;
      GWEN_Gui_ProgressLog(0,
                           GWEN_LoggerLevel_Notice,
                           tbuf);
      AH_User_SetPeerId(u, s);
    }
  }

  /* get crypt token */
  rv=AB_Banking_GetCryptToken(AH_HBCI_GetBankingApi(h),
                              AH_User_GetTokenType(u),
                              AH_User_GetTokenName(u),
                              &ct);
  if (rv) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "Could not get crypt token (%d)", rv);
    GWEN_Gui_ProgressLog(0,
                         GWEN_LoggerLevel_Error,
                         I18N("Error getting crypt token"));
    if (doLock)
      AB_Provider_EndExclUseUser(pro, u, 0);
    AH_Job_free(job);
    if (!nounmount)
      AB_Banking_ClearCryptTokenList(AH_HBCI_GetBankingApi(h));
    return rv;
  }

  /* open crypt token */
  rv=GWEN_Crypt_Token_Open(ct, 1, 0);
  if (rv) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "Could not open crypt token (%d)", rv);
    GWEN_Gui_ProgressLog(0,
                         GWEN_LoggerLevel_Error,
                         I18N("Error opening crypt token"));
    if (doLock)
      AB_Provider_EndExclUseUser(pro, u, 0);
    AH_Job_free(job);
    if (!nounmount)
      AB_Banking_ClearCryptTokenList(AH_HBCI_GetBankingApi(h));
    return rv;
  }

  /* get context */
  cctx=GWEN_Crypt_Token_GetContext(ct, AH_User_GetTokenContextId(u), 0);
  if (!cctx) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "User context not found on crypt token");
    GWEN_Gui_ProgressLog(0,
                         GWEN_LoggerLevel_Error,
                         I18N("User context not found on crypt token"));
    if (doLock)
      AB_Provider_EndExclUseUser(pro, u, 0);
    AH_Job_free(job);
    if (!nounmount)
      AB_Banking_ClearCryptTokenList(AH_HBCI_GetBankingApi(h));
    return GWEN_ERROR_NOT_FOUND;
  }
  else {
    GWEN_CRYPT_TOKEN_KEYINFO *ki;
    uint32_t kid;

    /* store sign key on token (if any) */
    kid=GWEN_Crypt_Token_Context_GetVerifyKeyId(cctx);
    ki=AH_Job_GetKeys_GetSignKeyInfo(job);
    if (kid && ki) {
      rv=GWEN_Crypt_Token_SetKeyInfo(ct,
                                     kid,
                                     ki,
                                     0);
      if (rv) {
        DBG_ERROR(AQHBCI_LOGDOMAIN, "Could not save key info (%d)", rv);
        GWEN_Gui_ProgressLog(0, GWEN_LoggerLevel_Error,
                             I18N("Error saving sign key"));
        if (doLock)
          AB_Provider_EndExclUseUser(pro, u, 0);
        AH_Job_free(job);
        if (!nounmount)
          AB_Banking_ClearCryptTokenList(AH_HBCI_GetBankingApi(h));
        return rv;
      }
      DBG_INFO(AQHBCI_LOGDOMAIN, "Sign key saved");
    }

    /* store crypt key on token */
    kid=GWEN_Crypt_Token_Context_GetEncipherKeyId(cctx);
    ki=AH_Job_GetKeys_GetCryptKeyInfo(job);
    if (kid && ki) {
      rv=GWEN_Crypt_Token_SetKeyInfo(ct,
                                     kid,
                                     ki,
                                     0);
      if (rv) {
        DBG_ERROR(AQHBCI_LOGDOMAIN, "Could not save key info (%d)", rv);
        GWEN_Gui_ProgressLog(0, GWEN_LoggerLevel_Error,
                             I18N("Error saving crypt key"));
        if (doLock)
          AB_Provider_EndExclUseUser(pro, u, 0);
        AH_Job_free(job);
        if (!nounmount)
          AB_Banking_ClearCryptTokenList(AH_HBCI_GetBankingApi(h));
        return rv;
      }
      DBG_INFO(AQHBCI_LOGDOMAIN, "Crypt key saved");
    }

    /* store auth key on token (if any) */
    kid=GWEN_Crypt_Token_Context_GetAuthVerifyKeyId(cctx);
    ki=AH_Job_GetKeys_GetAuthKeyInfo(job);
    if (kid && ki) {
      rv=GWEN_Crypt_Token_SetKeyInfo(ct,
                                     kid,
                                     ki,
                                     0);
      if (rv) {
        DBG_ERROR(AQHBCI_LOGDOMAIN, "Could not save key info (%d)", rv);
        GWEN_Gui_ProgressLog(0, GWEN_LoggerLevel_Error,
                             I18N("Error saving auth key"));
        if (doLock)
          AB_Provider_EndExclUseUser(pro, u, 0);
        AH_Job_free(job);
        if (!nounmount)
          AB_Banking_ClearCryptTokenList(AH_HBCI_GetBankingApi(h));
        return rv;
      }
      DBG_INFO(AQHBCI_LOGDOMAIN, "Auth key saved");
    }
  }

  AH_Job_free(job);
  GWEN_Gui_ProgressLog(0,
                       GWEN_LoggerLevel_Notice,
                       I18N("Keys saved."));

  /* unlock user */
  if (doLock) {
    rv=AB_Provider_EndExclUseUser(pro, u, 0);
    if (rv) {
      DBG_ERROR(AQHBCI_LOGDOMAIN, "Could not unlock user (%d)\n", rv);
      if (!nounmount)
        AB_Banking_ClearCryptTokenList(AH_HBCI_GetBankingApi(h));
      return rv;
    }
  }

  if (!nounmount)
    AB_Banking_ClearCryptTokenList(AH_HBCI_GetBankingApi(h));

  return 0;
}



int AH_Provider_SendUserKeys2(AB_PROVIDER *pro, AB_USER *u,
                              AB_IMEXPORTER_CONTEXT *ctx,
                              int withAuthKey,
                              int withProgress, int nounmount, int doLock)
{
  AB_BANKING *ab;
  AH_HBCI *h;
  AH_JOB *job;
  AH_OUTBOX *ob;
  int rv;
  GWEN_CRYPT_TOKEN *ct;
  uint32_t kid;
  const GWEN_CRYPT_TOKEN_CONTEXT *cctx;
  GWEN_CRYPT_TOKEN_KEYINFO *signKeyInfo=NULL;
  GWEN_CRYPT_TOKEN_KEYINFO *cryptKeyInfo=NULL;
  GWEN_CRYPT_TOKEN_KEYINFO *authKeyInfo=NULL;
  int mounted=0;

  assert(pro);
  assert(u);

  ab=AB_Provider_GetBanking(pro);
  assert(ab);

  h=AH_Provider_GetHbci(pro);
  assert(h);

  /* get crypt token */
  rv=AB_Banking_GetCryptToken(AH_HBCI_GetBankingApi(h),
                              AH_User_GetTokenType(u),
                              AH_User_GetTokenName(u),
                              &ct);
  if (rv) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "Could not get crypt token (%d)", rv);
    GWEN_Gui_ProgressLog(0,
                         GWEN_LoggerLevel_Error,
                         I18N("Error getting crypt token"));
    if (!nounmount)
      AB_Banking_ClearCryptTokenList(AH_HBCI_GetBankingApi(h));
    return rv;
  }

  /* open crypt token */
  rv=GWEN_Crypt_Token_Open(ct, 1, 0);
  if (rv) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "Could not open crypt token (%d)", rv);
    GWEN_Gui_ProgressLog(0,
                         GWEN_LoggerLevel_Error,
                         I18N("Error opening crypt token"));
    if (!nounmount)
      AB_Banking_ClearCryptTokenList(AH_HBCI_GetBankingApi(h));
    return rv;
  }

  /* get context */
  cctx=GWEN_Crypt_Token_GetContext(ct, AH_User_GetTokenContextId(u), 0);
  if (!cctx) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "User context not found on crypt token");
    GWEN_Gui_ProgressLog(0,
                         GWEN_LoggerLevel_Error,
                         I18N("User context not found on crypt token"));
    if (!nounmount)
      AB_Banking_ClearCryptTokenList(AH_HBCI_GetBankingApi(h));
    return GWEN_ERROR_NOT_FOUND;
  }

  /* get sign key info */
  kid=GWEN_Crypt_Token_Context_GetSignKeyId(cctx);
  if (kid) {
    uint32_t ctxSignKeyNum=GWEN_Crypt_Token_Context_GetSignKeyNum(cctx);
    const GWEN_CRYPT_TOKEN_KEYINFO *signKeyInfoCT=GWEN_Crypt_Token_GetKeyInfo(ct, kid,
                                                                              GWEN_CRYPT_TOKEN_KEYFLAGS_HASMODULUS |
                                                                              GWEN_CRYPT_TOKEN_KEYFLAGS_HASEXPONENT |
                                                                              GWEN_CRYPT_TOKEN_KEYFLAGS_HASKEYVERSION |
                                                                              GWEN_CRYPT_TOKEN_KEYFLAGS_HASKEYNUMBER,
                                                                              0);
    if (signKeyInfoCT==NULL) {
      DBG_ERROR(AQHBCI_LOGDOMAIN, "Sign key info not found on crypt token");
      GWEN_Gui_ProgressLog(0,
                           GWEN_LoggerLevel_Error,
                           I18N("Sign key info not found on crypt token"));
      if (!nounmount)
        AB_Banking_ClearCryptTokenList(AH_HBCI_GetBankingApi(h));
      return GWEN_ERROR_NOT_FOUND;
    }
    signKeyInfo=GWEN_Crypt_Token_KeyInfo_dup(signKeyInfoCT);
    if (ctxSignKeyNum) {
      GWEN_Crypt_Token_KeyInfo_SetKeyNumber(signKeyInfo, ctxSignKeyNum);
      GWEN_Crypt_Token_KeyInfo_SetKeyVersion(signKeyInfo, GWEN_Crypt_Token_Context_GetSignKeyVer(cctx));
    }

  }
  else {
    DBG_INFO(AQHBCI_LOGDOMAIN, "No sign key id");
  }

  /* get crypt key info */
  kid=GWEN_Crypt_Token_Context_GetDecipherKeyId(cctx);
  if (kid) {
    uint32_t ctxCryptKeyNum=GWEN_Crypt_Token_Context_GetDecipherKeyNum(cctx);
    const GWEN_CRYPT_TOKEN_KEYINFO *cryptKeyInfoCT=GWEN_Crypt_Token_GetKeyInfo(ct, kid,
                                                                               GWEN_CRYPT_TOKEN_KEYFLAGS_HASMODULUS |
                                                                               GWEN_CRYPT_TOKEN_KEYFLAGS_HASEXPONENT |
                                                                               GWEN_CRYPT_TOKEN_KEYFLAGS_HASKEYVERSION |
                                                                               GWEN_CRYPT_TOKEN_KEYFLAGS_HASKEYNUMBER,
                                                                               0);
    if (cryptKeyInfoCT==NULL) {
      DBG_ERROR(AQHBCI_LOGDOMAIN, "Crypt key info not found on crypt token");
      GWEN_Gui_ProgressLog(0,
                           GWEN_LoggerLevel_Error,
                           I18N("Crypt key info not found on crypt token"));
      if (!nounmount)
        AB_Banking_ClearCryptTokenList(AH_HBCI_GetBankingApi(h));
      return GWEN_ERROR_NOT_FOUND;
    }
    cryptKeyInfo=GWEN_Crypt_Token_KeyInfo_dup(cryptKeyInfoCT);
    if (ctxCryptKeyNum) {
      GWEN_Crypt_Token_KeyInfo_SetKeyNumber(cryptKeyInfo, ctxCryptKeyNum);
      GWEN_Crypt_Token_KeyInfo_SetKeyVersion(cryptKeyInfo, GWEN_Crypt_Token_Context_GetDecipherKeyVer(cctx));
    }
  }
  else {
    DBG_INFO(AQHBCI_LOGDOMAIN, "No decipher key id");
  }

  /* get auth sign key info */
  if (withAuthKey) {
    kid=GWEN_Crypt_Token_Context_GetAuthSignKeyId(cctx);
    if (kid) {
      uint32_t ctxAuthKeyNum=GWEN_Crypt_Token_Context_GetAuthSignKeyNum(cctx);
      const GWEN_CRYPT_TOKEN_KEYINFO *authKeyInfoCT=GWEN_Crypt_Token_GetKeyInfo(ct, kid,
                                                                                GWEN_CRYPT_TOKEN_KEYFLAGS_HASMODULUS |
                                                                                GWEN_CRYPT_TOKEN_KEYFLAGS_HASEXPONENT |
                                                                                GWEN_CRYPT_TOKEN_KEYFLAGS_HASKEYVERSION |
                                                                                GWEN_CRYPT_TOKEN_KEYFLAGS_HASKEYNUMBER,
                                                                                0);
      if (authKeyInfoCT==NULL) {
        DBG_ERROR(AQHBCI_LOGDOMAIN, "Auth key info not found on crypt token");
        GWEN_Gui_ProgressLog(0,
                             GWEN_LoggerLevel_Error,
                             I18N("Auth key info not found on crypt token"));
        if (!nounmount)
          AB_Banking_ClearCryptTokenList(AH_HBCI_GetBankingApi(h));
        return GWEN_ERROR_NOT_FOUND;
      }
      authKeyInfo=GWEN_Crypt_Token_KeyInfo_dup(authKeyInfoCT);
      if (ctxAuthKeyNum) {
        GWEN_Crypt_Token_KeyInfo_SetKeyNumber(authKeyInfo, ctxAuthKeyNum);
        GWEN_Crypt_Token_KeyInfo_SetKeyVersion(authKeyInfo, GWEN_Crypt_Token_Context_GetAuthSignKeyVer(cctx));
      }
    }
    else {
      DBG_INFO(AQHBCI_LOGDOMAIN, "No auth key id");
    }
  }

  /* create job */
  job=AH_Job_SendKeys_new(pro, u, cryptKeyInfo, signKeyInfo, authKeyInfo);

  /* free keyinfos */
  if (signKeyInfo) {
    GWEN_Crypt_Token_KeyInfo_free(signKeyInfo);
  }
  if (cryptKeyInfo) {
    GWEN_Crypt_Token_KeyInfo_free(cryptKeyInfo);
  }
  if (authKeyInfo) {
    GWEN_Crypt_Token_KeyInfo_free(authKeyInfo);
  }

  if (!job) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "Job not supported, should not happen");
    GWEN_Gui_ProgressLog(0,
                         GWEN_LoggerLevel_Error,
                         I18N("Job not supported, should not happen"));
    if (!nounmount && mounted)
      AB_Banking_ClearCryptTokenList(AH_HBCI_GetBankingApi(h));
    return GWEN_ERROR_GENERIC;
  }
  AH_Job_AddSigner(job, AB_User_GetUserId(u));

  /* enqueue job */
  ob=AH_Outbox_new(pro);
  AH_Outbox_AddJob(ob, job);

  /* execute queue */
  rv=AH_Outbox_Execute(ob, ctx, withProgress, 0, doLock);
  AH_Outbox_free(ob);
  if (rv) {
    GWEN_Gui_ProgressLog(0,
                         GWEN_LoggerLevel_Error,
                         I18N("Could not execute outbox."));
    AH_Job_free(job);
    if (!nounmount && mounted)
      AB_Banking_ClearCryptTokenList(AH_HBCI_GetBankingApi(h));
    return rv;
  }

  /* check result */
  if (AH_Job_HasErrors(job)) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "Job has errors");
    GWEN_Gui_ProgressLog(0,
                         GWEN_LoggerLevel_Error,
                         I18N("Job contains errors."));
    AH_Job_free(job);
    if (!nounmount && mounted)
      AB_Banking_ClearCryptTokenList(AH_HBCI_GetBankingApi(h));
    return GWEN_ERROR_GENERIC;
  }
  else {
    rv=AH_Job_Commit(job, doLock);
    if (rv) {
      DBG_ERROR(AQHBCI_LOGDOMAIN, "Could not commit result.\n");
      GWEN_Gui_ProgressLog(0,
                           GWEN_LoggerLevel_Error,
                           I18N("Could not commit result"));
      AH_Job_free(job);
      if (!nounmount && mounted)
        AB_Banking_ClearCryptTokenList(AH_HBCI_GetBankingApi(h));
      return rv;
    }
  }

  GWEN_Gui_ProgressLog(0,
                       GWEN_LoggerLevel_Notice,
                       I18N("Keys sent"));

  AH_Job_free(job);
  if (!nounmount && mounted)
    AB_Banking_ClearCryptTokenList(AH_HBCI_GetBankingApi(h));

  return 0;
}



int AH_Provider_SendUserKeys(AB_PROVIDER *pro, AB_USER *u,
                             AB_IMEXPORTER_CONTEXT *ctx,
                             int withProgress, int nounmount, int doLock)
{
  return AH_Provider_SendUserKeys2(pro, u, ctx, 0, withProgress, nounmount, doLock);
}



int AH_Provider_ChangeUserKeys(AB_PROVIDER *pro, AB_USER *u, GWEN_DB_NODE *args, int withProgress, int nounmount,
                               int doLock)
{
  int res=0;
  uint8_t canceled=0;
  AH_JOB *job=NULL;
  AB_IMEXPORTER_CONTEXT *ctx=NULL;

  assert(u);

  job=AH_Job_ChangeKeys_new(pro, u, args, &canceled);
  if (!job) {
    res = -2;
    if (!canceled) {
      DBG_ERROR(AQHBCI_LOGDOMAIN, "Unexplainable, 'AH_Job_ChangeKeys_new' not supported.");
      GWEN_Gui_ProgressLog(0, GWEN_LoggerLevel_Error, I18N("Unexplainable, 'AH_Job_ChangeKeys_new' not supported."));
      res = -1;
    }
    if (canceled == 2)
      res = -1;
  }

  if (!res) {
    ctx = AB_ImExporterContext_new();
    if (!ctx) {
      DBG_ERROR(AQHBCI_LOGDOMAIN, "Error getting ctx.");
      res = -1;
    }
  }
  if (ctx) {
    AH_OUTBOX *ob = AH_Outbox_new(pro);
    if (!ob)
      GWEN_Gui_ProgressLog(0, GWEN_LoggerLevel_Error, I18N("Allocate outbox failed."));
    else {
      AH_Job_AddSigner(job, AB_User_GetUserId(u));
      AH_Outbox_AddJob(ob, job);

      GWEN_Gui_ProgressLog(0, GWEN_LoggerLevel_Info, I18N("Fetching serverkeys."));
      res = AH_Outbox_Execute(ob, ctx, withProgress, 0, doLock);
      AH_Outbox_free(ob);
      if (res)
        GWEN_Gui_ProgressLog(0, GWEN_LoggerLevel_Error, I18N("Could not execute outbox."));
      DBG_INFO(AQHBCI_LOGDOMAIN, "res %d, status %d.", res, AH_Job_GetStatus(job));
      if (res || (AH_Job_GetStatus(job) == AH_JobStatusError))
        res = -1;
    }
    AB_ImExporterContext_free(ctx);
  }

  res = AH_Job_ChangeKeys_finish(pro, job, res);

  if (job)
    AH_Job_free(job);

  AB_Banking_ClearCryptTokenList(AH_HBCI_GetBankingApi(AH_Provider_GetHbci(pro)));

  return (res == -2) ? 0 : res;
}



int AH_Provider_GetCert(AB_PROVIDER *pro,
                        AB_USER *u,
                        int withProgress, int nounmount, int doLock)
{
  AB_BANKING *ab;
  AH_HBCI *h;
  int rv;
  AH_DIALOG *dialog;
  uint32_t pid;

  assert(pro);
  assert(u);

  ab=AB_Provider_GetBanking(pro);
  assert(ab);

  h=AH_Provider_GetHbci(pro);
  assert(h);

  pid=GWEN_Gui_ProgressStart(GWEN_GUI_PROGRESS_ALLOW_EMBED |
                             GWEN_GUI_PROGRESS_SHOW_PROGRESS |
                             GWEN_GUI_PROGRESS_SHOW_ABORT,
                             I18N("Getting Certificate"),
                             I18N("We are now asking the server for its "
                                  "SSL certificate"),
                             GWEN_GUI_PROGRESS_NONE,
                             0);
  /* first try */
  dialog=AH_Dialog_new(u, pro);
  assert(dialog);
  rv=AH_Dialog_TestServer_Https(dialog);
  AH_Dialog_Disconnect(dialog);
  AH_Dialog_free(dialog);

  if (rv) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "Could not connect to server (%d)", rv);
    GWEN_Gui_ProgressLog(pid,
                         GWEN_LoggerLevel_Error,
                         I18N("Could not connect to server"));
    GWEN_Gui_ProgressEnd(pid);
    return rv;
  }

  GWEN_Gui_ProgressLog(pid,
                       GWEN_LoggerLevel_Error,
                       I18N("Got certificate"));
  GWEN_Gui_ProgressEnd(pid);

  return 0;
}



int AH_Provider_GetItanModes(AB_PROVIDER *pro, AB_USER *u,
                             AB_IMEXPORTER_CONTEXT *ctx,
                             int withProgress, int nounmount, int doLock)
{
  AB_BANKING *ab;
  AH_HBCI *h;
  AH_JOB *job;
  AH_OUTBOX *ob;
  int rv;
  const int *tm;
  char tbuf[256];

  assert(pro);
  assert(u);

  ab=AB_Provider_GetBanking(pro);
  assert(ab);

  h=AH_Provider_GetHbci(pro);
  assert(h);

  if (doLock) {
    rv=AB_Provider_BeginExclUseUser(pro, u);
    if (rv<0) {
      DBG_INFO(AQHBCI_LOGDOMAIN,
               "Could not lock customer [%s] (%d)",
               AB_User_GetCustomerId(u), rv);
      snprintf(tbuf, sizeof(tbuf)-1,
               I18N("Could not lock user %s (%d)"),
               AB_User_GetUserId(u), rv);
      tbuf[sizeof(tbuf)-1]=0;
      GWEN_Gui_ProgressLog(0,
                           GWEN_LoggerLevel_Error,
                           tbuf);
      return rv;
    }
  }

  job=AH_Job_GetItanModes_new(pro, u);
  if (!job) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "Job not supported, should not happen");
    if (doLock)
      AB_Provider_EndExclUseUser(pro, u, 1);
    return GWEN_ERROR_GENERIC;
  }
  AH_Job_AddSigner(job, AB_User_GetUserId(u));

  ob=AH_Outbox_new(pro);
  AH_Outbox_AddJob(ob, job);
  rv=AH_Outbox_Execute(ob, ctx, withProgress, 1, 0);
  AH_Outbox_free(ob);
  if (rv) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "Could not execute outbox.");
    if (doLock)
      AB_Provider_EndExclUseUser(pro, u, 1);
    AH_Job_free(job);
    if (!nounmount)
      AB_Banking_ClearCryptTokenList(AH_HBCI_GetBankingApi(h));
    return rv;
  }

  tm=AH_Job_GetItanModes_GetModes(job);
  if (tm[0]==-1) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "No iTAN modes reported");
    GWEN_Gui_ProgressLog(0,
                         GWEN_LoggerLevel_Error,
                         I18N("No iTAN modes reported."));
    if (doLock)
      AB_Provider_EndExclUseUser(pro, u, 1);
    AH_Job_free(job);
    if (!nounmount)
      AB_Banking_ClearCryptTokenList(AH_HBCI_GetBankingApi(h));
    return GWEN_ERROR_NO_DATA;
  }

  /* we have received tan methods, so there was a 3920 response. In this
   * special case we need to apply the job data, because otherwise we couldn't
   * fully connect to the server next time.
   */
  rv=AH_Job_Commit(job, 0);
  if (rv) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "Could not commit result.\n");
    GWEN_Gui_ProgressLog(0,
                         GWEN_LoggerLevel_Error,
                         I18N("Could not commit result to the system"));
    if (doLock)
      AB_Provider_EndExclUseUser(pro, u, 1);
    AH_Job_free(job);
    if (!nounmount)
      AB_Banking_ClearCryptTokenList(AH_HBCI_GetBankingApi(h));
    return rv;
  }

  if (doLock) {
    rv=AB_Provider_EndExclUseUser(pro, u, 0);
    if (rv<0) {
      DBG_INFO(AQHBCI_LOGDOMAIN,
               "Could not unlock customer [%s] (%d)",
               AB_User_GetCustomerId(u), rv);
      snprintf(tbuf, sizeof(tbuf)-1,
               I18N("Could not unlock user %s (%d)"),
               AB_User_GetUserId(u), rv);
      tbuf[sizeof(tbuf)-1]=0;
      GWEN_Gui_ProgressLog(0,
                           GWEN_LoggerLevel_Error,
                           tbuf);
      AB_Provider_EndExclUseUser(pro, u, 1);
      AH_Job_free(job);
      if (!nounmount)
        AB_Banking_ClearCryptTokenList(AH_HBCI_GetBankingApi(h));
      return rv;
    }
  }

  AH_Job_free(job);

  if (!nounmount)
    AB_Banking_ClearCryptTokenList(AH_HBCI_GetBankingApi(h));

  return 0;
}



int AH_Provider_ChangePin(AB_PROVIDER *pro, AB_USER *u,
                          AB_IMEXPORTER_CONTEXT *ctx,
                          int withProgress, int nounmount, int doLock)
{
  AB_BANKING *ab;
  AH_HBCI *h;
  AH_JOB *job;
  AH_OUTBOX *ob;
  int rv;
  char pwbuf[32];

  assert(pro);
  assert(u);

  ab=AB_Provider_GetBanking(pro);
  assert(ab);

  h=AH_Provider_GetHbci(pro);
  assert(h);

  memset(pwbuf, 0, sizeof(pwbuf));
  rv=GWEN_Gui_InputBox(GWEN_GUI_INPUT_FLAGS_NUMERIC |
                       GWEN_GUI_INPUT_FLAGS_CONFIRM,
                       I18N("Enter New Banking PIN"),
                       I18N("Please enter a new banking PIN.\n"
                            "You must only enter numbers, not letters.\n"
                            "<html>"
                            "<p>"
                            "Please enter a new banking PIN."
                            "</p>"
                            "<p>"
                            "You must only enter numbers, not letters."
                            "</p>"
                            "</html>"),
                       pwbuf,
                       0, 8, 0);
  if (rv<0) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }
  job=AH_Job_ChangePin_new(pro, u, pwbuf);
  if (!job) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "Job not supported, should not happen");
    return GWEN_ERROR_GENERIC;
  }
  AH_Job_AddSigner(job, AB_User_GetUserId(u));

  ob=AH_Outbox_new(pro);
  AH_Outbox_AddJob(ob, job);

  rv=AH_Outbox_Execute(ob, ctx, withProgress, nounmount, doLock);
  AH_Outbox_free(ob);
  if (rv) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "Could not execute outbox.\n");
    AH_Job_free(job);
    if (!nounmount)
      AB_Banking_ClearCryptTokenList(AH_HBCI_GetBankingApi(h));
    return rv;
  }

  if (AH_Job_HasErrors(job)) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "Job has errors");
    // TODO: show errors
    AH_Job_free(job);
    if (!nounmount)
      AB_Banking_ClearCryptTokenList(AH_HBCI_GetBankingApi(h));
    return GWEN_ERROR_GENERIC;
  }
  else {
    rv=AH_Job_Commit(job, doLock);
    if (rv) {
      DBG_ERROR(AQHBCI_LOGDOMAIN, "Could not commit result.\n");
      AH_Job_free(job);
      if (!nounmount)
        AB_Banking_ClearCryptTokenList(AH_HBCI_GetBankingApi(h));
      return rv;
    }
  }

  AH_Job_free(job);

  if (!nounmount)
    AB_Banking_ClearCryptTokenList(AH_HBCI_GetBankingApi(h));

  return 0;
}



int AH_Provider_UnblockPin(AB_PROVIDER *pro,
                           AB_USER *u,
                           AB_IMEXPORTER_CONTEXT *ctx,
                           int withProgress, int nounmount, int doLock)
{
  AB_BANKING *ab;
  AH_HBCI *h;
  AH_OUTBOX *ob;
  int rv;
  AH_JOB *job;

  assert(pro);

  ab=AB_Provider_GetBanking(pro);
  assert(ab);

  h=AH_Provider_GetHbci(pro);
  assert(h);

  ob=AH_Outbox_new(pro);

  /* TODO: store user to free it later */
  job=AH_Job_UnblockPin_new(pro, u);
  if (!job) {
    DBG_WARN(AQHBCI_LOGDOMAIN, "Job not supported with this account");
    AH_Outbox_free(ob);
    return GWEN_ERROR_GENERIC;
  }
  AH_Job_AddSigner(job, AB_User_GetUserId(u));

  AH_Outbox_AddJob(ob, job);
  AH_Job_free(job);

  rv=AH_Outbox_Execute(ob, ctx, withProgress, nounmount, doLock);
  if (rv) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "Could not execute outbox.\n");
    AH_Outbox_free(ob);
    if (!nounmount)
      AB_Banking_ClearCryptTokenList(AH_HBCI_GetBankingApi(h));
    return rv;
  }

  AH_Outbox_free(ob);

  if (!nounmount)
    AB_Banking_ClearCryptTokenList(AH_HBCI_GetBankingApi(h));

  return 0;
}



int AH_Provider_GetAccountSepaInfo(AB_PROVIDER *pro,
                                   AB_ACCOUNT *a,
                                   AB_IMEXPORTER_CONTEXT *ctx,
                                   int withProgress, int nounmount, int doLock)
{
  AB_BANKING *ab;
  AH_HBCI *h;
  AH_OUTBOX *ob;
  uint32_t uid;
  int rv;

  assert(pro);

  ab=AB_Provider_GetBanking(pro);
  assert(ab);

  h=AH_Provider_GetHbci(pro);
  assert(h);


  ob=AH_Outbox_new(pro);

  uid=AB_Account_GetUserId(a);
  if (uid==0) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "No user for this account");
  }
  else {
    AB_USER *u;

    rv=AB_Provider_GetUser(pro, uid, 1, 1, &u);
    if (rv<0) {
      DBG_ERROR(AQHBCI_LOGDOMAIN, "Unknown user for this account");
    }
    else {
      AH_JOB *job;

      /* TODO: store user to free it later */
      job=AH_Job_GetAccountSepaInfo_new(pro, u, a);
      if (!job) {
        DBG_WARN(AQHBCI_LOGDOMAIN, "Job not supported with this account");
        AH_Outbox_free(ob);
        return GWEN_ERROR_GENERIC;
      }
      AH_Job_AddSigner(job, AB_User_GetUserId(u));
      AH_Outbox_AddJob(ob, job);
      AH_Job_free(job);
    }
  }

  rv=AH_Outbox_Execute(ob, ctx, withProgress, nounmount, doLock);
  if (rv) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "Could not execute outbox.\n");
    AH_Outbox_free(ob);
    if (!nounmount)
      AB_Banking_ClearCryptTokenList(AH_HBCI_GetBankingApi(h));
    return rv;
  }

  AH_Outbox_free(ob);

  if (!nounmount)
    AB_Banking_ClearCryptTokenList(AH_HBCI_GetBankingApi(h));

  return 0;
}

int AH_Provider_GetTargetAccount(AB_PROVIDER *pro,
                                   AB_ACCOUNT *a,
                                   AB_IMEXPORTER_CONTEXT *ctx,
                                   int withProgress, int nounmount, int doLock)
{
  AB_BANKING *ab;
  AH_HBCI *h;
  AH_OUTBOX *ob;
  uint32_t uid;
  int rv;

  assert(pro);

  ab=AB_Provider_GetBanking(pro);
  assert(ab);

  h=AH_Provider_GetHbci(pro);
  assert(h);


  ob=AH_Outbox_new(pro);

  uid=AB_Account_GetUserId(a);
  if (uid==0) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "No user for this account");
  }
  else {
    AB_USER *u;

    rv=AB_Provider_GetUser(pro, uid, 1, 1, &u);
    if (rv<0) {
      DBG_ERROR(AQHBCI_LOGDOMAIN, "Unknown user for this account");
    }
    else {
      AH_JOB *job;

      /* TODO: store user to free it later */
      job=AH_Job_GetTargetAccount_new(pro, u, a);
      if (!job) {
        DBG_WARN(AQHBCI_LOGDOMAIN, "Job not supported with this account");
        AH_Outbox_free(ob);
        return GWEN_ERROR_GENERIC;
      }
      AH_Job_AddSigner(job, AB_User_GetUserId(u));
      AH_Outbox_AddJob(ob, job);
      AH_Job_free(job);
    }
  }

  rv=AH_Outbox_Execute(ob, ctx, withProgress, nounmount, doLock);
  if (rv) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "Could not execute outbox.\n");
    AH_Outbox_free(ob);
    if (!nounmount)
      AB_Banking_ClearCryptTokenList(AH_HBCI_GetBankingApi(h));
    return rv;
  }

  AH_Outbox_free(ob);

  if (!nounmount)
    AB_Banking_ClearCryptTokenList(AH_HBCI_GetBankingApi(h));

  return 0;
}



