/***************************************************************************
 $RCSfile$
 -------------------
 cvs         : $Id$
 begin       : Mon Mar 01 2004
 copyright   : (C) 2004 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "bankinfoplugin_p.h"

#include <gwenhywfar/debug.h>
#include <gwenhywfar/misc.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>


GWEN_INHERIT_FUNCTIONS(AB_BANKINFO_PLUGIN)
GWEN_LIST_FUNCTIONS(AB_BANKINFO_PLUGIN, AB_BankInfoPlugin)
GWEN_LIST2_FUNCTIONS(AB_BANKINFO_PLUGIN, AB_BankInfoPlugin)



AB_BANKINFO_PLUGIN *AB_BankInfoPlugin_new(const char *country){
  AB_BANKINFO_PLUGIN *bip;

  assert(country);
  GWEN_NEW_OBJECT(AB_BANKINFO_PLUGIN, bip);
  GWEN_INHERIT_INIT(AB_BANKINFO_PLUGIN, bip);
  GWEN_LIST_INIT(AB_BANKINFO_PLUGIN, bip);
  bip->usage=1;

  bip->country=strdup(country);

  return bip;
}



void AB_BankInfoPlugin_free(AB_BANKINFO_PLUGIN *bip){
  if (bip) {
    assert(bip->usage);
    if (--(bip->usage)==0) {
      GWEN_INHERIT_FINI(AB_BANKINFO_PLUGIN, bip);

      GWEN_Plugin_free(bip->plugin);
      free(bip->country);

      GWEN_LIST_FINI(AB_BANKINFO_PLUGIN, bip);
      GWEN_FREE_OBJECT(bip);
    }
  }
}



void AB_BankInfoPlugin_Attach(AB_BANKINFO_PLUGIN *bip){
  assert(bip);
  assert(bip->usage);
  bip->usage++;
}



const char *AB_BankInfoPlugin_GetCountry(const AB_BANKINFO_PLUGIN *bip){
  assert(bip);
  assert(bip->usage);
  return bip->country;
}


AB_BANKINFO *AB_BankInfoPlugin_GetBankInfo(AB_BANKINFO_PLUGIN *bip,
                                           const char *branchId,
                                           const char *bankId){
  assert(bip);
  assert(bip->usage);
  if (bip->getBankInfoFn)
    return bip->getBankInfoFn(bip, branchId, bankId);

  DBG_INFO(AQBANKING_LOGDOMAIN, "GetBankInfo function not set");
  return 0;
}



int AB_BankInfoPlugin_GetBankInfoByTemplate(AB_BANKINFO_PLUGIN *bip,
                                            AB_BANKINFO *tbi,
                                            AB_BANKINFO_LIST2 *bl){
  assert(bip);
  assert(bip->usage);
  if (bip->getBankInfoByTemplateFn)
    return bip->getBankInfoByTemplateFn(bip, tbi, bl);

  DBG_INFO(AQBANKING_LOGDOMAIN, "GetBankInfoByTemplate function not set");
  return AB_ERROR_NOT_SUPPORTED;
}



AB_BANKINFO_CHECKRESULT
AB_BankInfoPlugin_CheckAccount(AB_BANKINFO_PLUGIN *bip,
                               const char *branchId,
                               const char *bankId,
                               const char *accountId){
  assert(bip);
  assert(bip->usage);
  if (bip->checkAccountFn)
    return bip->checkAccountFn(bip, branchId, bankId, accountId);

  DBG_INFO(AQBANKING_LOGDOMAIN, "CheckAccount function not set");
  return AB_BankInfoCheckResult_UnknownResult;
}



void AB_BankInfoPlugin_SetGetBankInfoFn(AB_BANKINFO_PLUGIN *bip,
                                        AB_BANKINFOPLUGIN_GETBANKINFO_FN f){
  assert(bip);
  assert(bip->usage);
  bip->getBankInfoFn=f;
}



void AB_BankInfoPlugin_SetGetBankInfoByTemplateFn(AB_BANKINFO_PLUGIN *bip,
                                                  AB_BANKINFOPLUGIN_GETBANKINFOBYTMPLATE_FN f){
  assert(bip);
  assert(bip->usage);
  bip->getBankInfoByTemplateFn=f;
}



void AB_BankInfoPlugin_SetCheckAccountFn(AB_BANKINFO_PLUGIN *bip,
                                         AB_BANKINFOPLUGIN_CHECKACCOUNT_FN f){
  assert(bip);
  assert(bip->usage);
  bip->checkAccountFn=f;
}



void AB_BankInfoPlugin_SetPlugin(AB_BANKINFO_PLUGIN *bip,
                                 GWEN_PLUGIN *pl) {
  assert(bip);
  assert(bip->usage);

  bip->plugin=pl;
}






