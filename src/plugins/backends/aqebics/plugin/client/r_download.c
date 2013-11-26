

#include "msg/msg.h"
#include "msg/keys.h"
#include "msg/zip.h"
#include "msg/xml.h"
#include "user_l.h"

#include <gwenhywfar/base64.h>



int EBC_Provider_XchgDownloadRequest(AB_PROVIDER *pro,
				     GWEN_HTTP_SESSION *sess,
				     AB_USER *u,
				     const char *requestType,
				     GWEN_BUFFER *targetBuffer,
				     int withReceipt,
				     const GWEN_TIME *fromTime,
				     const GWEN_TIME *toTime) {
  EBC_PROVIDER *dp;
  const char *s;

  assert(pro);
  dp=GWEN_INHERIT_GETDATA(AB_PROVIDER, EBC_PROVIDER, pro);
  assert(dp);

  s=EBC_User_GetProtoVersion(u);
  if (!(s && *s))
    s="H002";
  if (strcasecmp(s, "H002")==0)
    return EBC_Provider_XchgDownloadRequest_H002(pro, sess, u, requestType, targetBuffer,
						 withReceipt,
						 fromTime, toTime);
  else if (strcasecmp(s, "H003")==0)
    return EBC_Provider_XchgDownloadRequest_H003(pro, sess, u, requestType, targetBuffer,
						 withReceipt,
						 fromTime, toTime);
  else {
    DBG_ERROR(AQEBICS_LOGDOMAIN, "Proto version [%s] not supported", s);
    return GWEN_ERROR_INTERNAL;
  }
}






