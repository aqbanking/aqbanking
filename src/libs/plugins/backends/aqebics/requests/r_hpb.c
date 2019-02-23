/***************************************************************************
    begin       : Mon Mar 01 2004
    copyright   : (C) 2018 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#ifdef HAVE_CONFIG_H
# include <config.h>
#endif


#include "r_hpb_l.h"

#include "aqebics_l.h"
#include "user_l.h"

#include <gwenhywfar/gui.h>
#include <gwenhywfar/httpsession.h>


int EBC_Provider_XchgHpbRequest(AB_PROVIDER *pro, GWEN_HTTP_SESSION *sess, AB_USER *u)
{
  const char *s;

  s=EBC_User_GetProtoVersion(u);
  if (!(s && *s))
    s="H002";
  if (strcasecmp(s, "H002")==0)
    return EBC_Provider_XchgHpbRequest_H002(pro, sess, u);
  else if (strcasecmp(s, "H003")==0)
    return EBC_Provider_XchgHpbRequest_H003(pro, sess, u);
  else if (strcasecmp(s, "H004")==0)
    return EBC_Provider_XchgHpbRequest_H004(pro, sess, u);
  else {
    DBG_ERROR(AQEBICS_LOGDOMAIN, "Proto version [%s] not supported", s);
    return GWEN_ERROR_INTERNAL;
  }
}







