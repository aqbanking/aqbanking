/***************************************************************************
    begin       : Mon Mar 01 2004
    copyright   : (C) 2020 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifndef AH_PROVIDER_TAN_H
#define AH_PROVIDER_TAN_H


#include "aqhbci/tan/tanmethod.h"

#include <aqbanking/backendsupport/provider.h>
#include <aqbanking/backendsupport/user.h>



int AH_Provider_InputTanWithChallenge(AB_PROVIDER *pro,
                                      AB_USER *u,
                                      const AH_TAN_METHOD *tanMethodDescription,
                                      const char *sChallenge,
                                      const char *sChallengeHhd,
                                      char *passwordBuffer,
                                      int passwordMinLen,
                                      int passwordMaxLen);



#endif

