/***************************************************************************
    begin       : Sun Jun 02 2019
    copyright   : (C) 2019 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifndef AH_TAN_MECHANISM_H
#define AH_TAN_MECHANISM_H


#include <gwenhywfar/inherit.h>



#ifdef __cplusplus
extern "C" {
#endif


typedef struct AH_TAN_MECHANISM AH_TAN_MECHANISM;


GWEN_INHERIT_FUNCTION_DEFS(AH_TAN_MECHANISM)


#ifdef __cplusplus
} /* __cplusplus */
#endif


#include <aqhbci/banking/user.h>



#ifdef __cplusplus
extern "C" {
#endif



/* virtual functions */
typedef int (*AH_TAN_MECHANISM_GETTAN_FN)(AH_TAN_MECHANISM *tanMechanism,
                                          AB_USER *u,
                                          const char *title,
                                          const char *text,
                                          const uint8_t *challengePtr,
                                          uint32_t challengeLen,
                                          char *passwordBuffer,
                                          int passwordMinLen,
                                          int passwordMaxLen);



/**
 * Select a TAN input mechanism for the given tan method description (or use preselected mechanism).
 *
 * @param tanMethod selected TAN method for which to determine the input mechanis to use
 * @param preselectedId tan mechanism id selected by user (see @ref AB_BANKING_TANMETHOD_TEXT and following)
 */
AH_TAN_MECHANISM *AH_TanMechanism_Factory(const AH_TAN_METHOD *tanMethod, int preselectedId);



AH_TAN_MECHANISM *AH_TanMechanism_new(const AH_TAN_METHOD *tanMethod, int tanMethodId);
void AH_TanMechanism_free(AH_TAN_MECHANISM *tanMechanism);


const AH_TAN_METHOD *AH_TanMechanism_GetTanMethod(const AH_TAN_MECHANISM *tanMechanism);
int AH_TanMechanism_GetTanMethodId(const AH_TAN_MECHANISM *tanMechanism);


int AH_TanMechanism_GetTan(AH_TAN_MECHANISM *tanMechanism,
                           AB_USER *u,
                           const char *title,
                           const char *text,
                           const uint8_t *challengePtr,
                           uint32_t challengeLen,
                           char *passwordBuffer,
                           int passwordMinLen,
                           int passwordMaxLen);



void AH_TanMechanism_SetGetTanFn(AH_TAN_MECHANISM *tanMechanism, AH_TAN_MECHANISM_GETTAN_FN fn);



#ifdef __cplusplus
} /* __cplusplus */
#endif


#endif


