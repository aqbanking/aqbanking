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

#ifndef AQBANKING_BANKINFO_DE_P_H
#define AQBANKING_BANKINFO_DE_P_H

#include <aqbanking/bankinfoplugin_be.h>
#include <aqbanking/banking.h>

#include <ktoblzcheck.h>


typedef struct AB_BANKINFO_PLUGIN_DE AB_BANKINFO_PLUGIN_DE;
struct AB_BANKINFO_PLUGIN_DE {
  AB_BANKING *banking;
  AccountNumberCheck *checker;
  GWEN_DB_NODE *dbData;
};

void AB_BankInfoPluginDE_FreeData(void *bp, void *p);

AB_BANKINFO *AB_BankInfoPluginDE_GetBankInfo(AB_BANKINFO_PLUGIN *bip,
                                             const char *branchId,
                                             const char *bankId);

AB_BANKINFO_CHECKRESULT
AB_BankInfoPluginDE_CheckAccount(AB_BANKINFO_PLUGIN *bip,
                                 const char *branchId,
                                 const char *bankId,
                                 const char *accountId);


#endif /* AQBANKING_BANKINFO_DE_P_H */




