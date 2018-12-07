

#include "msg/msg.h"
#include "msg/keys.h"
#include "user_l.h"

#include <gwenhywfar/base64.h>



int EBC_Provider_XchgPubRequest(AB_PROVIDER *pro,
				GWEN_HTTP_SESSION *sess,
				AB_USER *u,
				const char *signVersion) {
  const char *s;

  s=EBC_User_GetProtoVersion(u);
  if (!(s && *s))
    s="H002";
  if (strcasecmp(s, "H002")==0)
    return EBC_Provider_XchgPubRequest_H002(pro, sess, u, signVersion);
  else if (strcasecmp(s, "H003")==0)
    return EBC_Provider_XchgPubRequest_H003(pro, sess, u, signVersion);
  else {
    DBG_ERROR(AQEBICS_LOGDOMAIN, "Proto version [%s] not supported", s);
    return GWEN_ERROR_INTERNAL;
  }
}







