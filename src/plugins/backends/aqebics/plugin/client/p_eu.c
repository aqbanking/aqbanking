/***************************************************************************
    begin       : Wed May 14 2008
    copyright   : (C) 2008 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#include <gwenhywfar/mdigest.h>



int EBC_Provider_MkEuCryptZipDoc(AB_PROVIDER *pro,
				 AB_USER *u,
				 const char *requestType,
				 const uint8_t *pMsg,
				 uint32_t lMsg,
				 GWEN_CRYPT_KEY *skey,
				 GWEN_BUFFER *sbuf) {
  const char *s;
  int rv;

  s=EBC_User_GetSignVersion(u);
  if (!(s && *s))
    s="A004";
  if (strcasecmp(s, "A004")==0)
    rv=EBC_Provider_MkEuCryptZipDoc_A004(pro, u, requestType, pMsg, lMsg, skey, sbuf);
  else if (strcasecmp(s, "A005")==0)
    rv=EBC_Provider_MkEuCryptZipDoc_A005(pro, u, requestType, pMsg, lMsg, skey, sbuf);
  else {
    DBG_ERROR(AQEBICS_LOGDOMAIN, "Invalid sign version: [%s]", s);
    return GWEN_ERROR_INVALID;
  }

  if (rv<0) {
    DBG_INFO(AQEBICS_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }

  return rv;
}






