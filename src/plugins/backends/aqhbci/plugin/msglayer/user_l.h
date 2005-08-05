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

#ifndef AH_USER_L_H
#define AH_USER_L_H


#include <aqhbci/user.h>


GWEN_LIST_FUNCTION_DEFS(AH_USER, AH_User);



AH_HBCI *AH_User_GetHbci(const AH_USER *u);


AH_USER *AH_User_fromDb(AH_BANK *b, GWEN_DB_NODE *db);
int AH_User_toDb(const AH_USER *u, GWEN_DB_NODE *db);


#endif /* AH_USER_L_H */






