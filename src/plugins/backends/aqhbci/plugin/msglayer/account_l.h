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


#include <aqhbci/account.h>

GWEN_LIST_FUNCTION_DEFS(AH_ACCOUNT, AH_Account);
GWEN_INHERIT_FUNCTION_DEFS(AH_ACCOUNT);

AH_ACCOUNT *AH_Account_fromDb(AH_BANK *b, GWEN_DB_NODE *db);
int AH_Account_toDb(const AH_ACCOUNT *a, GWEN_DB_NODE *db);



#endif /* AH_ACCOUNT_L_H */


