/***************************************************************************
    begin       : Sat Sep 14 2019
    copyright   : (C) 2019 by Martin Preuss
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


static int _extractAndSetMimeTypeAndImageData(const uint8_t *challengePtr,
                                              uint32_t challengeLen,
                                              GWEN_DB_NODE *dbMethodParams);




/* implementation */


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
  return 0;
}



int _extractAndSetMimeTypeAndImageData(const uint8_t *challengePtr,
                                       uint32_t challengeLen,
                                       GWEN_DB_NODE *dbMethodParams)
{
  int len;
  const uint8_t *p;
  int tagLen;

  /* set bin:challenge, char:mimetype */

  p=challengePtr;
  len=challengeLen;


  /* read 1st tag: mimetype */
  tagLen=(p[0]<<8)+p[1];
  p+=2;
  len--;
  if (len<tagLen) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "invalid tag length %d (remaining data length: %d)", tagLen, len);
    return GWEN_ERROR_BAD_DATA;
  }
  if (tagLen) {
    char *mimeType;

    mimeType=(char*) malloc(tagLen+1);
    assert(mimeType);
    memmove(mimeType, p, tagLen);
    mimeType[tagLen]=0;

    DBG_ERROR(AQHBCI_LOGDOMAIN, "Image mimetype: \"%s\"", mimeType);
    GWEN_DB_SetCharValue(dbMethodParams, GWEN_DB_FLAGS_OVERWRITE_VARS, "mimeType", mimeType);
    free(mimeType);
    p+=tagLen;
    len-=tagLen;
  }
  else {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "Empty mimetype");
    return GWEN_ERROR_BAD_DATA;
  }


  /* read 2nd tag: image data */
  tagLen=(p[0]<<8)+p[1];
  p+=2;
  len--;
  if (len<tagLen) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "invalid tag length %d (remaining data length: %d)", tagLen, len);
    return GWEN_ERROR_BAD_DATA;
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






