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


#include "de_p.h"
#include <aqbanking/banking.h>
#include <gwenhywfar/debug.h>
#include <gwenhywfar/misc.h>
#include <gwenhywfar/inherit.h>

GWEN_INHERIT(AB_BANKINFO_PLUGIN, AB_BANKINFO_PLUGIN_DE);


AB_BANKINFO_PLUGIN *de_factory(AB_BANKING *ab, GWEN_DB_NODE *db){
  AB_BANKINFO_PLUGIN *bip;
  AB_BANKINFO_PLUGIN_DE *bde;

  bip=AB_BankInfoPlugin_new("de");
  GWEN_NEW_OBJECT(AB_BANKINFO_PLUGIN_DE, bde);
  GWEN_INHERIT_SETDATA(AB_BANKINFO_PLUGIN, AB_BANKINFO_PLUGIN_DE,
                       bip, bde, AB_BankInfoPluginDE_FreeData);

  bde->banking=ab;
  bde->dbData=db;
  bde->checker=AccountNumberCheck_new();
  if (!bde->checker) {
    DBG_ERROR(AQBANKING_LOGDOMAIN,
              "KtoBlzCheck returned an error");
    AB_BankInfoPlugin_free(bip);
    return 0;
  }
  AB_BankInfoPlugin_SetGetBankInfoFn(bip, AB_BankInfoPluginDE_GetBankInfo);
  AB_BankInfoPlugin_SetCheckAccountFn(bip, AB_BankInfoPluginDE_CheckAccount);
  return bip;
}



void AB_BankInfoPluginDE_FreeData(void *bp, void *p){
  AB_BANKINFO_PLUGIN_DE *bde;

  bde=(AB_BANKINFO_PLUGIN_DE*)p;

  if (bde->checker)
    AccountNumberCheck_delete(bde->checker);
  GWEN_FREE_OBJECT(bde);
}



AB_BANKINFO *AB_BankInfoPluginDE_GetBankInfo(AB_BANKINFO_PLUGIN *bip,
                                             const char *branchId,
                                             const char *bankId){
  AB_BANKINFO_PLUGIN_DE *bde;
  const AccountNumberCheck_Record *r;

  assert(bip);
  bde=GWEN_INHERIT_GETDATA(AB_BANKINFO_PLUGIN, AB_BANKINFO_PLUGIN_DE, bip);
  assert(bde);

  assert(bde->checker);
  r=AccountNumberCheck_findBank(bde->checker, bankId);
  if (r) {
    AB_BANKINFO *bi;

    bi=AB_BankInfo_new();
    AB_BankInfo_SetBranchId(bi, branchId);
    AB_BankInfo_SetBankId(bi, bankId);
    AB_BankInfo_SetBankName(bi, AccountNumberCheck_Record_bankName(r));
    AB_BankInfo_SetLocation(bi, AccountNumberCheck_Record_location(r));
    return bi;
  }
  DBG_INFO(AQBANKING_LOGDOMAIN, "Bank \"%s\" not found", bankId);
  return 0;
}



AB_BANKINFO_CHECKRESULT
AB_BankInfoPluginDE_CheckAccount(AB_BANKINFO_PLUGIN *bip,
                                 const char *branchId,
                                 const char *bankId,
                                 const char *accountId){
  AB_BANKINFO_PLUGIN_DE *bde;
  AccountNumberCheck_Result res;
  AB_BANKINFO_CHECKRESULT cr;

  assert(bankId);
  assert(accountId);

  assert(bip);
  bde=GWEN_INHERIT_GETDATA(AB_BANKINFO_PLUGIN, AB_BANKINFO_PLUGIN_DE, bip);
  assert(bde);

  assert(bde->checker);
  res=AccountNumberCheck_check(bde->checker,
                               bankId,
                               accountId);
  switch(res) {
  case 0:  cr=AB_BankInfoCheckResult_Ok; break;
  case 1:  cr=AB_BankInfoCheckResult_UnknownResult; break;
  case 2:  cr=AB_BankInfoCheckResult_NotOk; break;
  case 3:  cr=AB_BankInfoCheckResult_UnknownBank; break;
  default: cr=AB_BankInfoCheckResult_UnknownResult; break;
  } /* switch */

  return cr;
}




















