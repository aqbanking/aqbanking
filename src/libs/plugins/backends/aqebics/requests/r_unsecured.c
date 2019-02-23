/***************************************************************************
    begin       : Mon Mar 01 2004
    copyright   : (C) 2019 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/



#include "msg/msg.h"
#include "msg/keys.h"
#include "user_l.h"
#include "r_unsecured_l.h"


#include <gwenhywfar/base64.h>





EB_MSG *EBC_Provider_MkUnsecuredRequest(AB_PROVIDER *pro,
                                        AB_USER *u,
                                        const char *orderType,
                                        const char *orderAttribute,
                                        const char *orderData)
{
  const char *s;

  s=EBC_User_GetProtoVersion(u);
  if (!(s && *s))
    s="H004";
  if (strcasecmp(s, "H004")==0)
    return EBC_Provider_MkUnsecuredRequest_H004(pro, u, orderType, orderAttribute, orderData);
  else {
    DBG_ERROR(AQEBICS_LOGDOMAIN, "Proto version [%s] not supported", s);
    return NULL;
  }
}




