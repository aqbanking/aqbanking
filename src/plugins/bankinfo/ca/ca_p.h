/***************************************************************************
 $RCSfile$
                             -------------------
    cvs         : $Id: ca_p.h 1138 2007-01-22 15:05:39Z christian $
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
  GWEN_DB_NODE *dbData;
};


void GWENHYWFAR_CB AB_BankInfoPluginCA_FreeData(void *bp, void *p);


#endif /* AQBANKING_BANKINFO_CA_P_H */




