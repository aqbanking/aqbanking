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
#include "../generic/generic_l.h"

#include <aqbanking/banking.h>
#include <gwenhywfar/debug.h>
#include <gwenhywfar/misc.h>
#include <gwenhywfar/inherit.h>
#include <gwenhywfar/text.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>



GWEN_INHERIT(AB_BANKINFO_PLUGIN, AB_BANKINFO_PLUGIN_DE);

/* interface to gwens plugin loader */
GWEN_PLUGIN *bankinfo_de_factory(GWEN_PLUGIN_MANAGER *pm,
                                 const char *name,
                                 const char *fileName) {
  return GWEN_Plugin_new(pm, name, fileName);
}



/* interface to bankinfo plugin */
AB_BANKINFO_PLUGIN *de_factory(AB_BANKING *ab, GWEN_DB_NODE *db){
  AB_BANKINFO_PLUGIN *bip;
  AB_BANKINFO_PLUGIN_DE *bde;

  bip=AB_BankInfoPluginGENERIC_new(ab, db, "de");
  GWEN_NEW_OBJECT(AB_BANKINFO_PLUGIN_DE, bde);
  GWEN_INHERIT_SETDATA(AB_BANKINFO_PLUGIN, AB_BANKINFO_PLUGIN_DE,
                       bip, bde, AB_BankInfoPluginDE_FreeData);

  bde->banking=ab;
  bde->dbData=db;
#ifdef HAVE_KTOBLZCHECK
  bde->checker = AccountNumberCheck_new();
  if (!bde->checker) {
    DBG_ERROR(AQBANKING_LOGDOMAIN,
              "KtoBlzCheck returned an error");
    AB_BankInfoPlugin_free(bip);
    return 0;
  }
#endif
  AB_BankInfoPlugin_SetCheckAccountFn(bip, AB_BankInfoPluginDE_CheckAccount);
  return bip;
}



void AB_BankInfoPluginDE_FreeData(void *bp, void *p){
  AB_BANKINFO_PLUGIN_DE *bde;

  bde=(AB_BANKINFO_PLUGIN_DE*)p;

#ifdef HAVE_KTOBLZCHECK
  if (bde->checker)
    AccountNumberCheck_delete(bde->checker);
#endif
  GWEN_FREE_OBJECT(bde);
}



AB_BANKINFO_CHECKRESULT
AB_BankInfoPluginDE_CheckAccount(AB_BANKINFO_PLUGIN *bip,
                                 const char *branchId,
                                 const char *bankId,
                                 const char *accountId){
  AB_BANKINFO_PLUGIN_DE *bde;
#ifdef HAVE_KTOBLZCHECK
  AccountNumberCheck_Result res;
#endif
  AB_BANKINFO_CHECKRESULT cr;

  assert(bankId);
  assert(accountId);

  assert(bip);
  bde=GWEN_INHERIT_GETDATA(AB_BANKINFO_PLUGIN, AB_BANKINFO_PLUGIN_DE, bip);
  assert(bde);

#ifdef HAVE_KTOBLZCHECK
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
#else
  cr=AB_BankInfoCheckResult_UnknownResult;
#endif

  return cr;
}








