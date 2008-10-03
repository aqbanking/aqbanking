/***************************************************************************
    begin       : Mon Mar 01 2004
    copyright   : (C) 2004 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifndef AQBANKING_BANKINFO_US_P_H
#define AQBANKING_BANKINFO_US_P_H

#include <aqbanking/bankinfoplugin_be.h>
#include <aqbanking/banking.h>


typedef struct AB_BANKINFO_PLUGIN_US AB_BANKINFO_PLUGIN_US;
struct AB_BANKINFO_PLUGIN_US {
  AB_BANKING *banking;
};


static
void GWENHYWFAR_CB AB_BankInfoPluginUS_FreeData(void *bp, void *p);

static
AB_BANKINFO_PLUGIN *AB_Plugin_BankInfoUS_Factory(GWEN_PLUGIN *pl, AB_BANKING *ab);


AQBANKING_EXPORT
GWEN_PLUGIN *bankinfo_us_factory(GWEN_PLUGIN_MANAGER *pm,
                                 const char *name,
				 const char *fileName);


#endif /* AQBANKING_BANKINFO_US_P_H */




