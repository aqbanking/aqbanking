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


#ifndef AQBANKING_BANKING_L_H
#define AQBANKING_BANKING_L_H


#include <aqbanking/banking.h>


GWEN_DB_NODE *AB_Banking_GetWizzardData(AB_BANKING *ab,
                                        const AB_PROVIDER_WIZZARD *pw);

GWEN_TYPE_UINT32 AB_Banking_GetUniqueId(AB_BANKING *ab);


#endif /* AQBANKING_BANKING_L_H */
