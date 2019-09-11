/***************************************************************************
    begin       : Tue Jun 04 2019
    copyright   : (C) 2019 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "tan_text.h"


#include <gwenhywfar/misc.h>
#include <gwenhywfar/debug.h>
#include <gwenhywfar/text.h>
#include <gwenhywfar/gui.h>



/* forward declarations */

static int _getTan(AH_TAN_MECHANISM *tanMechanism,
                   AB_USER *u,
                   const char *title,
                   const char *text,
                   const uint8_t *challengePtr,
                   uint32_t challengeLen,
                   char *passwordBuffer,
                   int passwordMinLen,
                   int passwordMaxLen);




/* implementation */


AH_TAN_MECHANISM *AH_TanMechanism_Text_new(const AH_TAN_METHOD *tanMethod, int tanMethodId)
{
  AH_TAN_MECHANISM *tanMechanism;

  tanMechanism=AH_TanMechanism_new(tanMethod, tanMethodId);
  assert(tanMechanism);

  AH_TanMechanism_SetGetTanFn(tanMechanism, _getTan);
  return tanMechanism;
}



int _getTan(AH_TAN_MECHANISM *tanMechanism,
            AB_USER *u,
            const char *title,
            const char *text,
            const uint8_t *challengePtr,
            uint32_t challengeLen,
            char *passwordBuffer,
            int passwordMinLen,
            int passwordMaxLen)
{
  const AH_TAN_METHOD *tanMethod;
  GWEN_DB_NODE *dbMethodParams;
  GWEN_BUFFER *bufToken;
  GWEN_DB_NODE *dbTanMethod;
  int rv;

  tanMethod=AH_TanMechanism_GetTanMethod(tanMechanism);
  assert(tanMethod);

  dbMethodParams=GWEN_DB_Group_new("methodParams");

  GWEN_DB_SetIntValue(dbMethodParams, GWEN_DB_FLAGS_OVERWRITE_VARS,
                      "tanMethodId", AH_TanMechanism_GetTanMethodId(tanMechanism));

  dbTanMethod=GWEN_DB_GetGroup(dbMethodParams, GWEN_DB_FLAGS_OVERWRITE_GROUPS, "tanMethod");
  AH_TanMethod_toDb(tanMethod, dbTanMethod);

  bufToken=GWEN_Buffer_new(0, 256, 0, 1);
  AH_User_MkTanName(u, (const char *) challengePtr, bufToken);

  rv=GWEN_Gui_GetPassword(GWEN_GUI_INPUT_FLAGS_TAN | GWEN_GUI_INPUT_FLAGS_SHOW | GWEN_GUI_INPUT_FLAGS_DIRECT,
                          GWEN_Buffer_GetStart(bufToken),
                          title,
                          text,
                          passwordBuffer,
                          passwordMinLen,
                          passwordMaxLen,
                          GWEN_Gui_PasswordMethod_Text,
                          dbMethodParams,
                          0);
  if (rv<0) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
    GWEN_Buffer_free(bufToken);
    GWEN_DB_Group_free(dbMethodParams);
    return rv;
  }

  GWEN_Buffer_free(bufToken);
  GWEN_DB_Group_free(dbMethodParams);
  return 0;
}



