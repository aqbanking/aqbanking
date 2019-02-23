/***************************************************************************
    begin       : Mon Mar 01 2004
    copyright   : (C) 2019 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif


#include "r_upload_l.h"

#include "aqebics_l.h"
#include "msg/msg.h"
#include "msg/keys.h"
#include "msg/zip.h"
#include "msg/xml.h"
#include "user_l.h"

#include <gwenhywfar/base64.h>
#include <gwenhywfar/gui.h>
#include <gwenhywfar/httpsession.h>



int EBC_Provider_XchgUploadRequest(AB_PROVIDER *pro,
                                   GWEN_HTTP_SESSION *sess,
                                   AB_USER *u,
                                   const char *requestType,
                                   const uint8_t *pData,
                                   uint32_t lData)
{
  const char *s;

  /* exchange upload request */
  s=EBC_User_GetProtoVersion(u);
  if (!(s && *s))
    s="H002";
  if (strcasecmp(s, "H002")==0)
    return EBC_Provider_XchgUploadRequest_H002(pro, sess, u, requestType, pData, lData);
  else if (strcasecmp(s, "H003")==0)
    return EBC_Provider_XchgUploadRequest_H003(pro, sess, u, requestType, pData, lData);
  else {
    DBG_ERROR(AQEBICS_LOGDOMAIN, "Crypt version [%s] not supported", s);
    return GWEN_ERROR_INTERNAL;
  }
}






