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

#ifndef AH_RESULT_L_H
#define AH_RESULT_L_H

#include <gwenhywfar/misc.h>

#include <stdio.h>


typedef struct AH_RESULT AH_RESULT;


GWEN_LIST_FUNCTION_DEFS(AH_RESULT, AH_Result);


AH_RESULT *AH_Result_new(int code,
                         const char *text,
                         const char *ref,
                         const char *param,
                         int isMsgResult);
void AH_Result_free(AH_RESULT *r);

AH_RESULT *AH_Result_dup(const AH_RESULT *or);

int AH_Result_GetCode(const AH_RESULT *r);
const char *AH_Result_GetText(const AH_RESULT *r);
const char *AH_Result_GetRef(const AH_RESULT *r);
const char *AH_Result_GetParam(const AH_RESULT *r);

int AH_Result_IsError(const AH_RESULT *r);
int AH_Result_IsWarning(const AH_RESULT *r);
int AH_Result_IsInfo(const AH_RESULT *r);
int AH_Result_IsOk(const AH_RESULT *r);
int AH_Result_IsMsgResult(const AH_RESULT *r);

void AH_Result_Dump(const AH_RESULT *r, FILE *f, unsigned int insert);


#endif /* AH_RESULT_H */




