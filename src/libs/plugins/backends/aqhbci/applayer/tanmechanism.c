/***************************************************************************
    begin       : Sun Jun 02 2019
    copyright   : (C) 2019 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "tanmechanism_p.h"
#include "tan_chiptan_opt.h"

#include <gwenhywfar/misc.h>
#include <gwenhywfar/debug.h>
#include <gwenhywfar/text.h>



GWEN_INHERIT_FUNCTIONS(AH_TAN_MECHANISM)



AH_TAN_MECHANISM *AH_TanMechanism_new(const AH_TAN_METHOD *tanMethod)
{
  AH_TAN_MECHANISM *tanMechanism;

  GWEN_NEW_OBJECT(AH_TAN_MECHANISM, tanMechanism);
  GWEN_INHERIT_INIT(AH_TAN_MECHANISM, tanMechanism);

  if (tanMethod)
    tanMechanism->tanMethod=AH_TanMethod_dup(tanMethod);

  return tanMechanism;
}



void AH_TanMechanism_free(AH_TAN_MECHANISM *tanMechanism)
{
  if (tanMechanism) {
    GWEN_INHERIT_FINI(AH_TAN_MECHANISM, tanMechanism)
      if (tanMechanism->tanMethod)
        AH_TanMethod_free(tanMechanism->tanMethod);
    GWEN_FREE_OBJECT(tanMechanism);
  }
}



const AH_TAN_METHOD *AH_TanMechanism_GetTanMethod(const AH_TAN_MECHANISM *tanMechanism)
{
  assert(tanMechanism);
  return tanMechanism->tanMethod;
}



int AH_TanMechanism_GetTan(AH_TAN_MECHANISM *tanMechanism,
                           AB_USER *u,
                           const char *title,
                           const char *text,
                           const uint8_t *challengePtr,
                           uint32_t challengeLen,
                           char *passwordBuffer,
                           int passwordMinLen,
                           int passwordMaxLen)
{
  assert(tanMechanism);
  if (tanMechanism->getTanFn)
    return (tanMechanism->getTanFn)(tanMechanism, u, title, text, challengePtr, challengeLen,
                                    passwordBuffer, passwordMinLen, passwordMaxLen);
  else
    return GWEN_ERROR_NOT_IMPLEMENTED;
}





void AH_TanMechanism_SetGetTanFn(AH_TAN_MECHANISM *tanMechanism, AH_TAN_MECHANISM_GETTAN_FN fn)
{
  assert(tanMechanism);
  tanMechanism->getTanFn=fn;
}







AH_TAN_MECHANISM *AH_TanMechanism_Factory(const AH_TAN_METHOD *tanMethod)
{
  const char *methodId;

  methodId=AH_TanMethod_GetMethodId(tanMethod);
  if (methodId && *methodId) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "Looking for mechanism implementing method id \"%s\"", methodId);

    if (-1!=GWEN_Text_ComparePattern(methodId, "HHD1.*OPT", 0)) {
      /* chipTan optisch */
      DBG_ERROR(AQHBCI_LOGDOMAIN, "Using TAN mechanism \"chipTAN optisch\"");
      return AH_TanMechanism_ChipTanOpt_new(tanMethod);
    }
    else {
      DBG_ERROR(AQHBCI_LOGDOMAIN, "No tan mechanism for method id \"%s\" found.", methodId);
    }
  }
  else {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "Empty method id");
  }

  return NULL;
}




