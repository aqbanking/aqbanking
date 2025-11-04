/***************************************************************************
    begin       : Sat Oct 18 2025
    copyright   : (C) 2025 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "vop_result_p.h"




/* ------------------------------------------------------------------------------------------------
 * forward declarations
 * ------------------------------------------------------------------------------------------------
 */



/* ------------------------------------------------------------------------------------------------
 * code
 * ------------------------------------------------------------------------------------------------
 */

GWEN_LIST_FUNCTIONS(AH_VOP_RESULT, AH_VopResult);



AH_VOP_RESULT *AH_VopResult_new()
{
  AH_VOP_RESULT *vr;

  GWEN_NEW_OBJECT(AH_VOP_RESULT, vr);
  GWEN_LIST_INIT(AH_VOP_RESULT, vr);
  return vr;
}



void AH_VopResult_free(AH_VOP_RESULT *vr)
{
  if (vr) {
    GWEN_LIST_FINI(AH_VOP_RESULT, vr);
    GWEN_FREE_OBJECT(vr);
  }
}



const char *AH_VopResult_GetLocalBic(const AH_VOP_RESULT *vr)
{
  return vr?vr->localBic:NULL;
}



void AH_VopResult_SetLocalBic(AH_VOP_RESULT *vr, const char *s)
{
  if (vr) {
    free(vr->localBic);
    vr->localBic=s?strdup(s):NULL;
  }
}



const char *AH_VopResult_GetRemoteIban(const AH_VOP_RESULT *vr)
{
  return vr?vr->remoteIban:NULL;
}



void AH_VopResult_SetRemoteIban(AH_VOP_RESULT *vr, const char *s)
{
  if (vr) {
    free(vr->remoteIban);
    vr->remoteIban=s?strdup(s):NULL;
  }
}



const char *AH_VopResult_GetRemoteName(const AH_VOP_RESULT *vr)
{
  return vr?vr->remoteName:NULL;
}



void AH_VopResult_SetRemoteName(AH_VOP_RESULT *vr, const char *s)
{
  if (vr) {
    free(vr->remoteName);
    vr->remoteName=s?strdup(s):NULL;
  }
}



const char *AH_VopResult_GetAltRemoteName(const AH_VOP_RESULT *vr)
{
  return vr?vr->altRemoteName:NULL;
}



void AH_VopResult_SetAltRemoteName(AH_VOP_RESULT *vr, const char *s)
{
  if (vr) {
    free(vr->altRemoteName);
    vr->altRemoteName=s?strdup(s):NULL;
  }
}



int AH_VopResult_GetResult(const AH_VOP_RESULT *vr)
{
  return vr?vr->result:AH_VopResultCodeNone;
}



void AH_VopResult_SetResult(AH_VOP_RESULT *vr, int i)
{
  if (vr)
    vr->result=i;
}



const AH_VOP_RESULT *AH_VopResult_List_GetByIbanAndName(const AH_VOP_RESULT_LIST *vrList,
                                                        const char *sIban,
                                                        const char *sRemoteName)
{
  if (vrList) {
    const AH_VOP_RESULT *vr;

    vr=AH_VopResult_List_First(vrList);
    while(vr) {
      if ((sIban && vr->remoteIban && strcasecmp(vr->remoteIban, sIban)==0) &&
          (sRemoteName==NULL || (vr->remoteName && strcasecmp(vr->remoteName, sRemoteName)==0)))
        return vr;
      vr=AH_VopResult_List_Next(vr);
    }
  }

  return NULL;
}



int AH_VopResultCode_fromString(const char *sResult)
{
  if (strcasecmp(sResult, "RCVC")==0)
    return AH_VopResultCodeMatch;
  else if (strcasecmp(sResult, "RVMC")==0)
    return AH_VopResultCodeCloseMatch;
  else if (strcasecmp(sResult, "RVNM")==0)
    return AH_VopResultCodeNoMatch;
  else if (strcasecmp(sResult, "RVNA")==0)
    return AH_VopResultCodeNotAvailable;
  else if (strcasecmp(sResult, "PDNG")==0)
    return AH_VopResultCodePending;
  else
    return AH_VopResultCodeNone;

}



const char *AH_VopResultCode_toString(int i)
{
  switch(i) {
  case AH_VopResultCodeNone:         return "none";
  case AH_VopResultCodeMatch:        return "RCVC";
  case AH_VopResultCodeCloseMatch:   return "RVMC";
  case AH_VopResultCodeNoMatch:      return "RVNM";
  case AH_VopResultCodeNotAvailable: return "RVNA";
  case AH_VopResultCodePending:      return "PDNG";
  default:                           return "none";
  }
}



void AH_VopResult_Log(const AH_VOP_RESULT *vr, const char *logDomain, GWEN_LOGGER_LEVEL lv)
{
  if (vr) {
    GWEN_BUFFER *mbuf;
    const char *sResultText;

    sResultText=AH_VopResultCode_toString(vr->result);
    mbuf=GWEN_Buffer_new(0, 256, 0, 1);
    GWEN_Buffer_AppendArgs(mbuf,
                           "VOP Result: %s (BIC: %s, rIBAN: %s, rNAME: %s, aNAME: %s)",
                           sResultText,
                           (vr->localBic)?(vr->localBic):"<empty>",
                           (vr->remoteIban)?(vr->remoteIban):"<empty>",
                           (vr->remoteName)?(vr->remoteName):"<empty>",
                           (vr->altRemoteName)?(vr->altRemoteName):"<empty>");
    GWEN_Logger_Log(logDomain, lv, GWEN_Buffer_GetStart(mbuf));
    GWEN_Buffer_free(mbuf);
  }
}





