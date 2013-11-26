/***************************************************************************
    begin       : Thu May 15 2008
    copyright   : (C) 2008 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/



int EBC_Provider_EncryptData(AB_PROVIDER *pro,
			     AB_USER *u,
			     GWEN_CRYPT_KEY *skey,
			     const uint8_t *pData,
			     uint32_t lData,
			     GWEN_BUFFER *sbuf) {
  const char *s;

  s=EBC_User_GetCryptVersion(u);
  if (!(s && *s))
    s="E001";
  if (strcasecmp(s, "E001")==0)
    return EBC_Provider_EncryptData_E001(pro, skey, pData, lData, sbuf);
  else if (strcasecmp(s, "E002")==0)
    return EBC_Provider_EncryptData_E002(pro, skey, pData, lData, sbuf);
  else {
    DBG_ERROR(AQEBICS_LOGDOMAIN, "Version [%s] not supported", s);
    return GWEN_ERROR_BAD_DATA;
  }
}




int EBC_Provider_EncryptKey(AB_PROVIDER *pro,
			    AB_USER *u,
			    const GWEN_CRYPT_KEY *skey,
			    GWEN_BUFFER *sbuf) {
  const char *s;

  s=EBC_User_GetCryptVersion(u);
  if (!(s && *s))
    s="E001";
  if (strcasecmp(s, "E001")==0)
    return EBC_Provider_EncryptKey_E001(pro, u, skey, sbuf);
  else if (strcasecmp(s, "E002")==0)
    return EBC_Provider_EncryptKey_E002(pro, u, skey, sbuf);
  else {
    DBG_ERROR(AQEBICS_LOGDOMAIN, "Version [%s] not supported", s);
    return GWEN_ERROR_BAD_DATA;
  }
}




