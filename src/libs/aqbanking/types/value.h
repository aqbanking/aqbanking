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


#ifndef AQBANKING_VALUE_H
#define AQBANKING_VALUE_H

#include <gwenhywfar/db.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct AB_VALUE AB_VALUE;

AB_VALUE *AB_Value_new(double value, const char *currency);
AB_VALUE *AB_Value_dup(const AB_VALUE *v);
AB_VALUE *AB_Value_FromString(const char *s);
AB_VALUE *AB_Value_FromDb(GWEN_DB_NODE *db);
int AB_Value_ToDb(const AB_VALUE *v, GWEN_DB_NODE *db);

void AB_Value_free(AB_VALUE *v);

double AB_Value_GetValue(const AB_VALUE *v);
void AB_Value_SetValue(AB_VALUE *v, double d);

const char *AB_Value_GetCurrency(const AB_VALUE *v);
void AB_Value_SetCurrency(AB_VALUE *v, const char *s);

int AB_Value_IsValid(const AB_VALUE *v);

#ifdef __cplusplus
}
#endif

#endif /* AQBANKING_VALUE_H */


