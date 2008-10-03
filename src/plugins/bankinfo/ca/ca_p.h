/***************************************************************************
    begin       : Mon Mar 01 2004
    copyright   : (C) 2004 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifndef AQBANKING_BANKINFO_CA_P_H
#define AQBANKING_BANKINFO_CA_P_H

#include <aqbanking/bankinfoplugin_be.h>
#include <aqbanking/banking.h>


typedef struct AB_BANKINFO_PLUGIN_CA AB_BANKINFO_PLUGIN_CA;
struct AB_BANKINFO_PLUGIN_CA {
  AB_BANKING *banking;
};


static
void GWENHYWFAR_CB AB_BankInfoPluginCA_FreeData(void *bp, void *p);


static
AB_BANKINFO_PLUGIN *AB_Plugin_BankInfoCA_Factory(GWEN_PLUGIN *pl, AB_BANKING *ab);



AQBANKING_EXPORT
GWEN_PLUGIN *bankinfo_ca_factory(GWEN_PLUGIN_MANAGER *pm,
                                 const char *name,
                                 const char *fileName);


#endif /* AQBANKING_BANKINFO_CA_P_H */




