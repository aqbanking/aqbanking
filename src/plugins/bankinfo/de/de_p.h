/***************************************************************************
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

#ifdef HAVE_KTOBLZCHECK
# include <ktoblzcheck.h>
#endif



typedef struct AB_BANKINFO_PLUGIN_DE AB_BANKINFO_PLUGIN_DE;
struct AB_BANKINFO_PLUGIN_DE {
  AB_BANKING *banking;
#ifdef HAVE_KTOBLZCHECK
  AccountNumberCheck *checker;
#endif
};

static
void GWENHYWFAR_CB AB_BankInfoPluginDE_FreeData(void *bp, void *p);

static
AB_BANKINFO_PLUGIN *AB_Plugin_BankInfoDE_Factory(GWEN_PLUGIN *pl, AB_BANKING *ab);


static AB_BANKINFO_CHECKRESULT
AB_BankInfoPluginDE_CheckAccount(AB_BANKINFO_PLUGIN *bip,
                                 const char *branchId,
                                 const char *bankId,
                                 const char *accountId);

AQBANKING_EXPORT
GWEN_PLUGIN *bankinfo_de_factory(GWEN_PLUGIN_MANAGER *pm,
				 const char *name,
				 const char *fileName);

#endif /* AQBANKING_BANKINFO_DE_P_H */




