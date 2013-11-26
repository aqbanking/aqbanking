/***************************************************************************
    begin       : Tue Mar 04 2008
    copyright   : (C) 2008 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/




int EBC_Provider_SignMessage(AB_PROVIDER *pro,
			     EB_MSG *msg,
			     AB_USER *u,
                             xmlNodePtr node) {
  const char *s;

  s=EBC_User_GetAuthVersion(u);
  if (!(s && *s))
    s="X001";

  if (strcasecmp(s, "X001")==0)
    return EBC_Provider_SignMessage_X001(pro, msg, u, node);
  else if (strcasecmp(s, "X002")==0)
    return EBC_Provider_SignMessage_X002(pro, msg, u, node);
  else {
    DBG_ERROR(AQEBICS_LOGDOMAIN, "Version [%s] not supported", s);
    return GWEN_ERROR_BAD_DATA;
  }
}




