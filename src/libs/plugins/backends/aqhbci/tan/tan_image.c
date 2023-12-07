/***************************************************************************
    begin       : Sat Sep 14 2019
    copyright   : (C) 2023 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "tan_image.h"

#include <gwenhywfar/misc.h>
#include <gwenhywfar/debug.h>
#include <gwenhywfar/text.h>
#include <gwenhywfar/gui.h>



/* ------------------------------------------------------------------------------------------------
 * forward declarations
 * ------------------------------------------------------------------------------------------------
 */

static int _getTan(AH_TAN_MECHANISM *tanMechanism,
                   AB_USER *u,
                   const char *title,
                   const char *text,
                   const uint8_t *challengePtr,
                   uint32_t challengeLen,
                   char *passwordBuffer,
                   int passwordMinLen,
                   int passwordMaxLen);
static int _extractAndSetMimeTypeAndImageData(const uint8_t *challengePtr, uint32_t challengeLen, GWEN_DB_NODE *dbMethodParams);
static int _readTagHeader(const uint8_t **pBufferPointer, int *pBufferLen, int *pTagLen, int tagNum);
static void _readTagIntoDbAsString(const uint8_t *p, int tagLen, GWEN_DB_NODE *db, const char *varName);



/* ------------------------------------------------------------------------------------------------
 * implementations
 * ------------------------------------------------------------------------------------------------
 */

AH_TAN_MECHANISM *AH_TanMechanism_Image_new(const AH_TAN_METHOD *tanMethod, int tanMethodId)
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

  tanMethod=AH_TanMechanism_GetTanMethod(tanMechanism);
  assert(tanMethod);

  if (challengePtr && challengeLen) {
    GWEN_BUFFER *bufToken;
    GWEN_DB_NODE *dbTanMethod;
    GWEN_DB_NODE *dbMethodParams;
    int rv;

    dbMethodParams=GWEN_DB_Group_new("methodParams");

    GWEN_DB_SetIntValue(dbMethodParams, GWEN_DB_FLAGS_OVERWRITE_VARS,
                        "tanMethodId", AH_TanMechanism_GetTanMethodId(tanMechanism));

    dbTanMethod=GWEN_DB_GetGroup(dbMethodParams, GWEN_DB_FLAGS_OVERWRITE_GROUPS, "tanMethod");
    AH_TanMethod_toDb(tanMethod, dbTanMethod);

    rv=_extractAndSetMimeTypeAndImageData(challengePtr, challengeLen, dbMethodParams);
    if (rv<0) {
      DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
      GWEN_DB_Group_free(dbMethodParams);
      return rv;
    }


    bufToken=GWEN_Buffer_new(0, 256, 0, 1);
    AH_User_MkTanName(u, (const char *) challengePtr, bufToken);

    rv=GWEN_Gui_GetPassword(GWEN_GUI_INPUT_FLAGS_TAN | GWEN_GUI_INPUT_FLAGS_SHOW | GWEN_GUI_INPUT_FLAGS_DIRECT,
                            GWEN_Buffer_GetStart(bufToken),
                            title,
                            text,
                            passwordBuffer,
                            passwordMinLen,
                            passwordMaxLen,
                            GWEN_Gui_PasswordMethod_OpticalHHD,
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
  }
  else {
    int rv;

    rv=GWEN_Gui_GetPassword(GWEN_GUI_INPUT_FLAGS_TAN | GWEN_GUI_INPUT_FLAGS_SHOW | GWEN_GUI_INPUT_FLAGS_DIRECT,
                            "TAN",
                            title,
                            text,
                            passwordBuffer,
                            passwordMinLen,
                            passwordMaxLen,
                            GWEN_Gui_PasswordMethod_Text,
                            NULL,
                            0);
    if (rv<0) {
      DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
      return rv;
    }
  }

  return 0;
}



int _extractAndSetMimeTypeAndImageData(const uint8_t *challengePtr, uint32_t challengeLen, GWEN_DB_NODE *dbMethodParams)
{
  int len;
  const uint8_t *p;
  int tagLen;
  int rv;

  /* set bin:imageData, char:mimetype */
  p=challengePtr;
  len=challengeLen;

  /* read 1st tag: mimetype */
  rv=_readTagHeader(&p, &len, &tagLen, 1);
  if (rv<0) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }
  if (tagLen) {
    _readTagIntoDbAsString(p, tagLen, dbMethodParams, "mimeType");
    p+=tagLen;
    len-=tagLen;
  }
  else {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "Empty mimetype");
    return GWEN_ERROR_NO_DATA;
  }

  /* read 2nd tag: image data */
  rv=_readTagHeader(&p, &len, &tagLen, 2);
  if (rv<0) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }
  if (tagLen) {
    GWEN_DB_SetBinValue(dbMethodParams, GWEN_DB_FLAGS_OVERWRITE_VARS, "imageData", p, tagLen);
    p+=tagLen;
    len-=tagLen;
  }
  else {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "Empty image data");
    return GWEN_ERROR_BAD_DATA;
  }

  /* ignore rest if any */
  return 0;
}



int _readTagHeader(const uint8_t **pBufferPointer, int *pBufferLen, int *pTagLen, int tagNum)
{
  int len;
  const uint8_t *p;
  int tagLen;

  len=*pBufferLen;
  p=*pBufferPointer;
  if (len<2) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "TAG %d: invalid data length (remaining data length: %d)", tagNum, len);
    return GWEN_ERROR_BAD_DATA;
  }
  tagLen=(p[0]<<8)+p[1];
  p+=2;
  len-=2;
  if (len<tagLen) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "TAG %d: invalid tag length %d (remaining data length: %d)", tagNum, tagLen, len);
    return GWEN_ERROR_BAD_DATA;
  }
  *pBufferPointer=p;
  *pBufferLen=len;
  *pTagLen=tagLen;
  return 0;
}


void _readTagIntoDbAsString(const uint8_t *p, int tagLen, GWEN_DB_NODE *db, const char *varName)
{
  char *stringValue;
  int i;

  stringValue=(char *) malloc(tagLen+1);
  assert(stringValue);
  memmove(stringValue, p, tagLen);
  stringValue[tagLen]=0;

  for (i=(tagLen-1); i>0; i--) {
    if (stringValue[i]==32)
      stringValue[i]=0;
    else
      break;
  }

  DBG_INFO(AQHBCI_LOGDOMAIN, "%s: \"%s\"", varName, stringValue);
  GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS, varName, stringValue);
  free(stringValue);
}






