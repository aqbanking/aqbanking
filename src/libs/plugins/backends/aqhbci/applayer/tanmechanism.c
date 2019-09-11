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
#include "tan_text.h"

#include <gwenhywfar/misc.h>
#include <gwenhywfar/debug.h>
#include <gwenhywfar/text.h>



GWEN_INHERIT_FUNCTIONS(AH_TAN_MECHANISM)



typedef struct {
  const char *namePattern;
  int id;
} TAN_MAP_ENTRY;


static TAN_MAP_ENTRY _zkaNameMap[]= {
  {"HHD",       AB_BANKING_TANMETHOD_TEXT},
  {"mobileTAN", AB_BANKING_TANMETHOD_TEXT},
  {NULL,        0}
};



static TAN_MAP_ENTRY _methodIdMap[]= {
  {"smsTAN",     AB_BANKING_TANMETHOD_TEXT},
  {"iTAN",       AB_BANKING_TANMETHOD_TEXT},
  {"HHD1.*OPT",  AB_BANKING_TANMETHOD_CHIPTAN_OPTIC},
  {"HHD1.*USB",  AB_BANKING_TANMETHOD_CHIPTAN_USB},
  {"HHD1.*QR",   AB_BANKING_TANMETHOD_CHIPTAN_QR},

  {"HHD1.*",     AB_BANKING_TANMETHOD_TEXT}, /* fallback */
  {NULL,        0}
};




/* forward declarations */

static int _getTanMethodIdFromString(const char *name, const TAN_MAP_ENTRY *nameMap);
static int _getTanMethodIdForTanMethod(const AH_TAN_METHOD *tanMethod);





AH_TAN_MECHANISM *AH_TanMechanism_new(const AH_TAN_METHOD *tanMethod, int tanMethodId)
{
  AH_TAN_MECHANISM *tanMechanism;

  GWEN_NEW_OBJECT(AH_TAN_MECHANISM, tanMechanism);
  GWEN_INHERIT_INIT(AH_TAN_MECHANISM, tanMechanism);

  if (tanMethod)
    tanMechanism->tanMethod=AH_TanMethod_dup(tanMethod);

  tanMechanism->tanMethodId=tanMethodId;

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



int AH_TanMechanism_GetTanMethodId(const AH_TAN_MECHANISM *tanMechanism)
{
  assert(tanMechanism);
  return tanMechanism->tanMethodId;
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
  int id;
  AH_TAN_MECHANISM *tanMechanism=NULL;

  id=_getTanMethodIdForTanMethod(tanMethod);
  if (id==0) {
    int idFunction;
    const char *sZkaName;
    const char *sMethodId;

    idFunction=AH_TanMethod_GetFunction(tanMethod);
    sZkaName=AH_TanMethod_GetZkaTanName(tanMethod);
    sMethodId=AH_TanMethod_GetMethodId(tanMethod);

    DBG_ERROR(AQHBCI_LOGDOMAIN,
              "No tan mechanism for method %d: zkaName=%s methodId=%s found, trying simple text input.",
              idFunction,
              sZkaName?sZkaName:"<empty>",
              sMethodId?sMethodId:"<empty>");
#if 0
    if (1) {
      GWEN_DB_NODE *dbDump;

      dbDump=GWEN_DB_Group_new("TanMethod");
      AH_TanMethod_toDb(tanMethod, dbDump);
      GWEN_DB_Dump(dbDump, 2);
      GWEN_DB_Group_free(dbDump);
    }
#endif
  }

  switch (id) {
  case AB_BANKING_TANMETHOD_CHIPTAN_OPTIC:
    DBG_ERROR(AQHBCI_LOGDOMAIN, "Using TAN mechanism \"chipTAN optisch\"");
    tanMechanism=AH_TanMechanism_ChipTanOpt_new(tanMethod, id);
    break;
  case AB_BANKING_TANMETHOD_CHIPTAN_USB:
  case AB_BANKING_TANMETHOD_CHIPTAN_QR:
  case AB_BANKING_TANMETHOD_PHOTOTAN:
    break;
  case AB_BANKING_TANMETHOD_CHIPTAN:
  case AB_BANKING_TANMETHOD_TEXT:
  default:
    DBG_ERROR(AQHBCI_LOGDOMAIN, "Using TAN mechanism \"text\"");
    tanMechanism=AH_TanMechanism_Text_new(tanMethod, id);
    break;
  }

  if (tanMechanism==NULL) {
    int idFunction;
    const char *sZkaName;
    const char *sMethodId;

    idFunction=AH_TanMethod_GetFunction(tanMethod);
    sZkaName=AH_TanMethod_GetZkaTanName(tanMethod);
    sMethodId=AH_TanMethod_GetMethodId(tanMethod);

    DBG_ERROR(AQHBCI_LOGDOMAIN,
              "No tan mechanism for method %d: zkaName=%s methodId=%s created (%d).",
              idFunction,
              sZkaName?sZkaName:"<empty>",
              sMethodId?sMethodId:"<empty>",
              id);
    return NULL;
  }

  return tanMechanism;
}



int _getTanMethodIdForTanMethod(const AH_TAN_METHOD *tanMethod)
{
  const char *sName;

  /* try ZKA name (at least Sparkassen in Hamburg or Lower Saxony dont provide this field in their TAN description */
  sName=AH_TanMethod_GetZkaTanName(tanMethod);
  if (sName && *sName) {
    int id;

    id=_getTanMethodIdFromString(sName, _zkaNameMap);
    if (id>0)
      return id;
  }

  /* try method id */
  sName=AH_TanMethod_GetMethodId(tanMethod);
  if (sName && *sName) {
    int id;

    id=_getTanMethodIdFromString(sName, _methodIdMap);
    if (id>0)
      return id;
  }

  return 0;
}



int _getTanMethodIdFromString(const char *name, const TAN_MAP_ENTRY *nameMap)
{
  const TAN_MAP_ENTRY *currentMapEntry;

  currentMapEntry=nameMap;
  while (currentMapEntry->namePattern) {
    if (-1!=GWEN_Text_ComparePattern(name, currentMapEntry->namePattern, 0))
      return currentMapEntry->id;
    currentMapEntry++;
  }

  return 0;
}




