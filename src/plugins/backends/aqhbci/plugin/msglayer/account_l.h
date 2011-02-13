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


#ifndef AH_ACCOUNT_L_H
#define AH_ACCOUNT_L_H


#include "account.h"
#include "hbci_l.h"
#include <aqbanking/provider_be.h>


int AH_Account_Extend(AB_ACCOUNT *a, AB_PROVIDER *pro,
                      AB_PROVIDER_EXTEND_MODE um,
                      GWEN_DB_NODE *db);

AH_HBCI *AH_Account_GetHbci(const AB_ACCOUNT *a);


#endif /* AH_ACCOUNT_L_H */


