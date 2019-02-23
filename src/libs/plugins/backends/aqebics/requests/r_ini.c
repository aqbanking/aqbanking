/***************************************************************************
    begin       : Mon Mar 01 2004
    copyright   : (C) 2019 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#include "msg/msg.h"
#include "msg/keys.h"
#include "msg/zip.h"
#include "msg/xml.h"
#include "user_l.h"
#include "provider_l.h"
#include "aqebics_l.h"

#include <gwenhywfar/base64.h>
#include <gwenhywfar/gui.h>
#include <gwenhywfar/debug.h>

#include <aqbanking/httpsession.h>


#include "r_ini_h002.c"
#include "r_ini_h003.c"
#include "r_ini_h004.c"



int EBC_Provider_XchgIniRequest(AB_PROVIDER *pro,
                                GWEN_HTTP_SESSION *sess,
                                AB_USER *u)
{
  const char *s;

  s=EBC_User_GetProtoVersion(u);
  if (!(s && *s))
    s="H002";
  if (strcasecmp(s, "H002")==0)
    return EBC_Provider_XchgIniRequest_H002(pro, sess, u);
  else if (strcasecmp(s, "H003")==0)
    return EBC_Provider_XchgIniRequest_H003(pro, sess, u);
  else if (strcasecmp(s, "H004")==0)
    return EBC_Provider_XchgIniRequest_H004(pro, sess, u);
  else {
    DBG_ERROR(AQEBICS_LOGDOMAIN, "Proto version [%s] not supported", s);
    return GWEN_ERROR_INTERNAL;
  }
}







