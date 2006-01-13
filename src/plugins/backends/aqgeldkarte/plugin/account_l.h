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

#ifndef AG_ACCOUNT_L_H
#define AG_ACCOUNT_L_H

#include "account.h"
#include <aqbanking/account_be.h>
#include <aqbanking/provider_be.h>


void AG_Account_Extend(AB_ACCOUNT *a, AB_PROVIDER *pro,
                       AB_PROVIDER_EXTEND_MODE em);


#endif
