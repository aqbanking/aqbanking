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
#include <aqbanking/provider_be.h>
#include <gwenhywfar/msgengine.h>


void AH_User_Extend(AB_USER *u, AB_PROVIDER *pro,
                    AB_PROVIDER_EXTEND_MODE em);

AH_HBCI *AH_User_GetHbci(const AB_USER *u);
GWEN_MSGENGINE *AH_User_GetMsgEngine(const AB_USER *u);


#endif /* AH_USER_L_H */






