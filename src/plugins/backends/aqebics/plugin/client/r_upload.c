
#include "msg/msg.h"
#include "msg/keys.h"
#include "msg/zip.h"
#include "msg/xml.h"
#include "user_l.h"



int EBC_Provider_XchgUploadRequest(AB_PROVIDER *pro,
				   GWEN_HTTP_SESSION *sess,
				   AB_USER *u,
				   const char *requestType,
				   const uint8_t *pData,
				   uint32_t lData) {
  EBC_PROVIDER *dp;
  const char *s;

  assert(pro);
  dp=GWEN_INHERIT_GETDATA(AB_PROVIDER, EBC_PROVIDER, pro);
  assert(dp);

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






