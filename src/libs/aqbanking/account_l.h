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


#ifndef AQBANKING_ACCOUNT_L_H
#define AQBANKING_ACCOUNT_L_H

#include <gwenhywfar/misc.h>
#include <gwenhywfar/misc2.h>
#include <gwenhywfar/inherit.h>
#include <aqbanking/account_be.h>


void AB_Account_Attach(AB_ACCOUNT *acc);


int AB_Account_Update(AB_ACCOUNT *a);

AB_ACCOUNT *AB_Account_fromDbWithProvider(AB_BANKING *ab,
                                          GWEN_DB_NODE *db);

void AB_Account_SetProvider(AB_ACCOUNT *a, AB_PROVIDER *pro);


#endif /* AQBANKING_ACCOUNT_L_H */
