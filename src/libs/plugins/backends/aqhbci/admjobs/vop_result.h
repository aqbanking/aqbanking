/***************************************************************************
    begin       : Sat Oct 18 2025
    copyright   : (C) 2025 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifndef AH_VOP_RESULT_H
#define AH_VOP_RESULT_H


#include "aqhbci/aqhbci_l.h"
#include "aqhbci/joblayer/job_l.h"

#include <gwenhywfar/list.h>


typedef struct AH_VOP_RESULT AH_VOP_RESULT;
GWEN_LIST_FUNCTION_DEFS(AH_VOP_RESULT, AH_VopResult);


enum {
  AH_VopResultCodeNone=0,
  AH_VopResultCodeMatch,
  AH_VopResultCodeCloseMatch,
  AH_VopResultCodeNoMatch,
  AH_VopResultCodeNotAvailable,
  AH_VopResultCodePending
};


int AH_VopResultCode_fromString(const char *sResult);
const char *AH_VopResultCode_toString(int i);
const char *AH_VopResultCode_toGuiString(int i);



AH_VOP_RESULT *AH_VopResult_new();
void AH_VopResult_free(AH_VOP_RESULT *vr);


const char *AH_VopResult_GetLocalBic(const AH_VOP_RESULT *vr);
void AH_VopResult_SetLocalBic(AH_VOP_RESULT *vr, const char *s);

const char *AH_VopResult_GetRemoteIban(const AH_VOP_RESULT *vr);
void AH_VopResult_SetRemoteIban(AH_VOP_RESULT *vr, const char *s);

const char *AH_VopResult_GetRemoteName(const AH_VOP_RESULT *vr);
void AH_VopResult_SetRemoteName(AH_VOP_RESULT *vr, const char *s);

const char *AH_VopResult_GetAltRemoteName(const AH_VOP_RESULT *vr);
void AH_VopResult_SetAltRemoteName(AH_VOP_RESULT *vr, const char *s);

int AH_VopResult_GetResult(const AH_VOP_RESULT *vr);
void AH_VopResult_SetResult(AH_VOP_RESULT *vr, int i);

void AH_VopResult_Log(const AH_VOP_RESULT *vr, const char *logDomain, GWEN_LOGGER_LEVEL lv);


const AH_VOP_RESULT *AH_VopResult_List_GetByIbanAndName(const AH_VOP_RESULT_LIST *vrList,
                                                        const char *sIban,
                                                        const char *sRemoteName);

int AH_VopResult_List_HasOnlyMatches(const AH_VOP_RESULT_LIST *vrList);

#endif

