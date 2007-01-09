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

#ifndef AQBANKING_BANKINFO_AT_P_H
#define AQBANKING_BANKINFO_AT_P_H

#include <aqbanking/bankinfoplugin_be.h>
#include <aqbanking/banking.h>


typedef struct AB_BANKINFO_PLUGIN_AT AB_BANKINFO_PLUGIN_AT;
struct AB_BANKINFO_PLUGIN_AT {
  AB_BANKING *banking;
  GWEN_DB_NODE *dbData;
};


void GWENHYWFAR_CB AB_BankInfoPluginAT_FreeData(void *bp, void *p);


#endif /* AQBANKING_BANKINFO_AT_P_H */




