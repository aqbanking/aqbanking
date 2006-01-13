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

#ifndef AG_ACCOUNT_H
#define AG_ACCOUNT_H

#include <aqgeldkarte/aqgeldkarte.h>
#include <aqgeldkarte/provider.h>

#include <aqbanking/account_be.h>


#ifdef __cplusplus
extern "C" {
#endif


AQGELDKARTE_API
const char *AG_Account_GetCardId(const AB_ACCOUNT *acc);

AQGELDKARTE_API
void AG_Account_SetCardId(AB_ACCOUNT *acc, const char *s);







#ifdef __cplusplus
}
#endif


#endif
