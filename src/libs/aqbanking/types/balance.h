/***************************************************************************
 $RCSfile$
                             -------------------
    cvs         : $Id$
    begin       : Mon Apr 05 2004
    copyright   : (C) 2004 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#ifndef AQBANKING_BALANCE_H
#define AQBANKING_BALANCE_H

#include <gwenhywfar/gwentime.h>
#include <aqbanking/value.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct AB_BALANCE AB_BALANCE;

AB_BALANCE *AB_Balance_new(AB_VALUE *v, GWEN_TIME *t);
AB_BALANCE *AB_Balance_FromDb(GWEN_DB_NODE *db);
int AB_Balance_ToDb(const AB_BALANCE *b, GWEN_DB_NODE *db);

AB_BALANCE *AB_Balance_dup(const AB_BALANCE *b);
void AB_Balance_free(AB_BALANCE *b);

const AB_VALUE *AB_Balance_GetValue(const AB_BALANCE *b);
const GWEN_TIME *AB_Balance_GetTime(const AB_BALANCE *b);

#ifdef __cplusplus
}
#endif

#endif /* AQBANKING_BALANCE_H */


