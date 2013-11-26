

#include <gwenhywfar/url.h>
#include "user_l.h"
#include "msg/keys.h"



int EBC_Provider_Send_INI(AB_PROVIDER *pro, AB_USER *u, int doLock) {
  EBC_PROVIDER *dp;
  GWEN_HTTP_SESSION *sess;
  int rv;
  EBC_USER_STATUS ust;
  AB_BANKING *ab;

  assert(pro);
  dp=GWEN_INHERIT_GETDATA(AB_PROVIDER, EBC_PROVIDER, pro);
  assert(dp);

  ab=AB_Provider_GetBanking(pro);

  if (EBC_User_GetFlags(u) & EBC_USER_FLAGS_INI) {
    DBG_ERROR(AQEBICS_LOGDOMAIN, "INI already sent to the server");
    return GWEN_ERROR_INVALID;
  }

  ust=EBC_User_GetStatus(u);
  if (ust!=EBC_UserStatus_New &&
      ust!=EBC_UserStatus_Init1 &&
      ust!=EBC_UserStatus_Disabled) {
    DBG_ERROR(AQEBICS_LOGDOMAIN,
	      "Invalid status \"%s\" of user \"%s\"",
	      EBC_User_Status_toString(ust),
	      AB_User_GetUserId(u));
    return GWEN_ERROR_INVALID;
  }

  /* create and open session */
  sess=EBC_Dialog_new(pro, u);
  rv=GWEN_HttpSession_Init(sess);
  if (rv<0) {
    DBG_ERROR(AQEBICS_LOGDOMAIN, "Could not open session");
    GWEN_HttpSession_free(sess);
    return rv;
  }

  /* lock user */
  if (doLock) {
    rv=AB_Banking_BeginExclUseUser(ab, u);
    if (rv<0) {
      DBG_ERROR(AQEBICS_LOGDOMAIN, "Could not lock customer");
      GWEN_HttpSession_free(sess);
      return rv;
    }
  }

  /* exchange request and response */
  rv=EBC_Provider_XchgIniRequest(pro, sess, u);
  if (rv) {
    DBG_ERROR(AQEBICS_LOGDOMAIN,
              "Error exchanging INI request (%d)", rv);
    if (doLock)
      AB_Banking_EndExclUseUser(ab, u, 1);
    GWEN_HttpSession_free(sess);
    return rv;
  }

  /* unlock user */
  if (doLock) {
    rv=AB_Banking_EndExclUseUser(ab, u, 0);
    if (rv<0) {
      DBG_ERROR(AQEBICS_LOGDOMAIN, "Could not unlock customer");
      AB_Banking_EndExclUseUser(ab, u, 1);
      GWEN_HttpSession_free(sess);
      return rv;
    }
  }

  /* close and destroy session */
  GWEN_HttpSession_Fini(sess);
  GWEN_HttpSession_free(sess);

  return rv;
}



int EBC_Provider_Send_HIA(AB_PROVIDER *pro, AB_USER *u, int doLock) {
  EBC_PROVIDER *dp;
  GWEN_HTTP_SESSION *sess;
  int rv;
  EBC_USER_STATUS ust;
  AB_BANKING *ab;

  assert(pro);
  dp=GWEN_INHERIT_GETDATA(AB_PROVIDER, EBC_PROVIDER, pro);
  assert(dp);

  ab=AB_Provider_GetBanking(pro);

  if (EBC_User_GetFlags(u) & EBC_USER_FLAGS_HIA) {
    DBG_ERROR(AQEBICS_LOGDOMAIN, "HIA already sent to the server");
    return GWEN_ERROR_INVALID;
  }

  ust=EBC_User_GetStatus(u);
  if (ust!=EBC_UserStatus_New &&
      ust!=EBC_UserStatus_Init1 &&
      ust!=EBC_UserStatus_Disabled) {
    DBG_ERROR(AQEBICS_LOGDOMAIN,
	      "Invalid status \"%s\" of user \"%s\"",
	      EBC_User_Status_toString(ust),
	      AB_User_GetUserId(u));
    return GWEN_ERROR_INVALID;
  }

  /* create and open session */
  sess=EBC_Dialog_new(pro, u);
  rv=GWEN_HttpSession_Init(sess);
  if (rv<0) {
    DBG_ERROR(AQEBICS_LOGDOMAIN, "Could not open session");
    GWEN_HttpSession_free(sess);
    return rv;
  }

  /* lock user */
  if (doLock) {
    rv=AB_Banking_BeginExclUseUser(ab, u);
    if (rv<0) {
      DBG_ERROR(AQEBICS_LOGDOMAIN, "Could not lock customer");
      GWEN_HttpSession_free(sess);
      return rv;
    }
  }

  /* exchange request and response */
  rv=EBC_Provider_XchgHiaRequest(pro, sess, u);
  if (rv) {
    DBG_ERROR(AQEBICS_LOGDOMAIN,
	      "Error exchanging HIA request (%d)", rv);
    if (doLock)
      AB_Banking_EndExclUseUser(ab, u, 1);
    GWEN_HttpSession_free(sess);
    return rv;
  }

  /* unlock user */
  if (doLock) {
    rv=AB_Banking_EndExclUseUser(ab, u, 0);
    if (rv<0) {
      DBG_ERROR(AQEBICS_LOGDOMAIN, "Could not unlock customer");
      AB_Banking_EndExclUseUser(ab, u, 1);
      GWEN_HttpSession_free(sess);
      return rv;
    }
  }

  /* close and destroy session */
  GWEN_HttpSession_Fini(sess);
  GWEN_HttpSession_free(sess);

  return rv;
}



int EBC_Provider_Send_HPB(AB_PROVIDER *pro, AB_USER *u, int doLock) {
  EBC_PROVIDER *dp;
  GWEN_HTTP_SESSION *sess;
  int rv;
  EBC_USER_STATUS ust;
  AB_BANKING *ab;

  assert(pro);
  dp=GWEN_INHERIT_GETDATA(AB_PROVIDER, EBC_PROVIDER, pro);
  assert(dp);

  ab=AB_Provider_GetBanking(pro);

  ust=EBC_User_GetStatus(u);
  if (ust!=EBC_UserStatus_Init2 &&
      ust!=EBC_UserStatus_Enabled) {
    DBG_ERROR(AQEBICS_LOGDOMAIN,
	      "Invalid status \"%s\" of user \"%s\"",
	      EBC_User_Status_toString(ust),
	      AB_User_GetUserId(u));
    return GWEN_ERROR_INVALID;
  }

  /* create and open session */
  sess=EBC_Dialog_new(pro, u);
  rv=GWEN_HttpSession_Init(sess);
  if (rv<0) {
    DBG_ERROR(AQEBICS_LOGDOMAIN, "Could not open session");
    GWEN_HttpSession_free(sess);
    return rv;
  }

  /* lock user */
  if (doLock) {
    rv=AB_Banking_BeginExclUseUser(ab, u);
    if (rv<0) {
      DBG_ERROR(AQEBICS_LOGDOMAIN, "Could not lock customer");
      GWEN_HttpSession_free(sess);
      return rv;
    }
  }

  /* exchange request and response */
  rv=EBC_Provider_XchgHpbRequest(pro, sess, u);
  if (rv) {
    DBG_ERROR(AQEBICS_LOGDOMAIN,
	      "Error exchanging HPB request (%d)", rv);
    if (doLock)
      AB_Banking_EndExclUseUser(ab, u, 1);
    GWEN_HttpSession_free(sess);
    return rv;
  }

  /* unlock user */
  if (doLock) {
    rv=AB_Banking_EndExclUseUser(ab, u, 0);
    if (rv<0) {
      DBG_ERROR(AQEBICS_LOGDOMAIN, "Could not unlock customer");
      AB_Banking_EndExclUseUser(ab, u, 1);
      GWEN_HttpSession_free(sess);
      return rv;
    }
  }

  /* close and destroy session */
  GWEN_HttpSession_Fini(sess);
  GWEN_HttpSession_free(sess);

  return rv;
}



int EBC_Provider_Send_HPD(AB_PROVIDER *pro, AB_USER *u, int doLock) {
  EBC_PROVIDER *dp;
  GWEN_HTTP_SESSION *sess;
  int rv;
  EBC_USER_STATUS ust;
  AB_BANKING *ab;

  assert(pro);
  dp=GWEN_INHERIT_GETDATA(AB_PROVIDER, EBC_PROVIDER, pro);
  assert(dp);

  ab=AB_Provider_GetBanking(pro);

  ust=EBC_User_GetStatus(u);
  if (ust!=EBC_UserStatus_Enabled) {
    DBG_ERROR(AQEBICS_LOGDOMAIN,
	      "Invalid status \"%s\" of user \"%s\"",
	      EBC_User_Status_toString(ust),
	      AB_User_GetUserId(u));
    return GWEN_ERROR_INVALID;
  }

  /* create and open session */
  sess=EBC_Dialog_new(pro, u);
  rv=GWEN_HttpSession_Init(sess);
  if (rv<0) {
    DBG_ERROR(AQEBICS_LOGDOMAIN, "Could not open session");
    GWEN_HttpSession_free(sess);
    return rv;
  }

  /* lock user */
  if (doLock) {
    rv=AB_Banking_BeginExclUseUser(ab, u);
    if (rv<0) {
      DBG_ERROR(AQEBICS_LOGDOMAIN, "Could not lock customer");
      GWEN_HttpSession_free(sess);
      return rv;
    }
  }

  /* exchange request and response */
  rv=EBC_Provider_XchgHpdRequest(pro, sess, u);
  if (rv) {
    DBG_ERROR(AQEBICS_LOGDOMAIN,
	      "Error exchanging HPD request (%d)", rv);
    if (doLock)
      AB_Banking_EndExclUseUser(ab, u, 1);
    GWEN_HttpSession_free(sess);
    return rv;
  }

  /* unlock user */
  if (doLock) {
    rv=AB_Banking_EndExclUseUser(ab, u, 0);
    if (rv<0) {
      DBG_ERROR(AQEBICS_LOGDOMAIN, "Could not unlock customer");
      AB_Banking_EndExclUseUser(ab, u, 1);
      GWEN_HttpSession_free(sess);
      return rv;
    }
  }

  /* close and destroy session */
  GWEN_HttpSession_Fini(sess);
  GWEN_HttpSession_free(sess);

  return rv;
}



int EBC_Provider_Send_HKD(AB_PROVIDER *pro, AB_USER *u, int doLock) {
  EBC_PROVIDER *dp;
  GWEN_HTTP_SESSION *sess;
  int rv;
  EBC_USER_STATUS ust;
  AB_BANKING *ab;

  assert(pro);
  dp=GWEN_INHERIT_GETDATA(AB_PROVIDER, EBC_PROVIDER, pro);
  assert(dp);

  ab=AB_Provider_GetBanking(pro);

  ust=EBC_User_GetStatus(u);
  if (ust!=EBC_UserStatus_Enabled) {
    DBG_ERROR(AQEBICS_LOGDOMAIN,
	      "Invalid status \"%s\" of user \"%s\"",
	      EBC_User_Status_toString(ust),
	      AB_User_GetUserId(u));
    return GWEN_ERROR_INVALID;
  }

  /* create and open session */
  sess=EBC_Dialog_new(pro, u);
  rv=GWEN_HttpSession_Init(sess);
  if (rv<0) {
    DBG_ERROR(AQEBICS_LOGDOMAIN, "Could not open session");
    GWEN_HttpSession_free(sess);
    return rv;
  }

  /* lock user */
  if (doLock) {
    rv=AB_Banking_BeginExclUseUser(ab, u);
    if (rv<0) {
      DBG_ERROR(AQEBICS_LOGDOMAIN, "Could not lock customer");
      GWEN_HttpSession_free(sess);
      return rv;
    }
  }

  /* exchange request and response */
  rv=EBC_Provider_XchgHkdRequest(pro, sess, u);
  if (rv) {
    DBG_ERROR(AQEBICS_LOGDOMAIN,
	      "Error exchanging HKD request (%d)", rv);
    if (doLock)
      AB_Banking_EndExclUseUser(ab, u, 1);
    GWEN_HttpSession_free(sess);
    return rv;
  }

  /* unlock user */
  if (doLock) {
    rv=AB_Banking_EndExclUseUser(ab, u, 0);
    if (rv<0) {
      DBG_ERROR(AQEBICS_LOGDOMAIN, "Could not unlock customer");
      AB_Banking_EndExclUseUser(ab, u, 1);
      GWEN_HttpSession_free(sess);
      return rv;
    }
  }

  /* close and destroy session */
  GWEN_HttpSession_Fini(sess);
  GWEN_HttpSession_free(sess);

  return rv;
}



int EBC_Provider_Send_HTD(AB_PROVIDER *pro, AB_USER *u, int doLock) {
  EBC_PROVIDER *dp;
  GWEN_HTTP_SESSION *sess;
  int rv;
  EBC_USER_STATUS ust;
  AB_BANKING *ab;

  assert(pro);
  dp=GWEN_INHERIT_GETDATA(AB_PROVIDER, EBC_PROVIDER, pro);
  assert(dp);

  ab=AB_Provider_GetBanking(pro);

  ust=EBC_User_GetStatus(u);
  if (ust!=EBC_UserStatus_Enabled) {
    DBG_ERROR(AQEBICS_LOGDOMAIN,
	      "Invalid status \"%s\" of user \"%s\"",
	      EBC_User_Status_toString(ust),
	      AB_User_GetUserId(u));
    return GWEN_ERROR_INVALID;
  }

  /* create and open session */
  sess=EBC_Dialog_new(pro, u);
  rv=GWEN_HttpSession_Init(sess);
  if (rv<0) {
    DBG_ERROR(AQEBICS_LOGDOMAIN, "Could not open session");
    GWEN_HttpSession_free(sess);
    return rv;
  }

  /* lock user */
  if (doLock) {
    rv=AB_Banking_BeginExclUseUser(ab, u);
    if (rv<0) {
      DBG_ERROR(AQEBICS_LOGDOMAIN, "Could not lock customer");
      GWEN_HttpSession_free(sess);
      return rv;
    }
  }

  /* exchange request and response */
  rv=EBC_Provider_XchgHtdRequest(pro, sess, u);
  if (rv) {
    DBG_ERROR(AQEBICS_LOGDOMAIN,
	      "Error exchanging HTD request (%d)", rv);
    if (doLock)
      AB_Banking_EndExclUseUser(ab, u, 1);
    GWEN_HttpSession_free(sess);
    return rv;
  }

  /* unlock user */
  if (doLock) {
    rv=AB_Banking_EndExclUseUser(ab, u, 0);
    if (rv<0) {
      DBG_ERROR(AQEBICS_LOGDOMAIN, "Could not unlock customer");
      AB_Banking_EndExclUseUser(ab, u, 1);
      GWEN_HttpSession_free(sess);
      return rv;
    }
  }

  /* close and destroy session */
  GWEN_HttpSession_Fini(sess);
  GWEN_HttpSession_free(sess);

  return rv;
}



int EBC_Provider_Send_PUB(AB_PROVIDER *pro, AB_USER *u, const char *signVersion, int doLock) {
  EBC_PROVIDER *dp;
  GWEN_HTTP_SESSION *sess;
  int rv;
  AB_BANKING *ab;

  assert(pro);
  dp=GWEN_INHERIT_GETDATA(AB_PROVIDER, EBC_PROVIDER, pro);
  assert(dp);

  ab=AB_Provider_GetBanking(pro);

  /* create and open session */
  sess=EBC_Dialog_new(pro, u);
  rv=GWEN_HttpSession_Init(sess);
  if (rv<0) {
    DBG_ERROR(AQEBICS_LOGDOMAIN, "Could not open session");
    GWEN_HttpSession_free(sess);
    return rv;
  }

  /* lock user */
  if (doLock) {
    rv=AB_Banking_BeginExclUseUser(ab, u);
    if (rv<0) {
      DBG_ERROR(AQEBICS_LOGDOMAIN, "Could not lock customer");
      GWEN_HttpSession_free(sess);
      return rv;
    }
  }

  /* exchange request and response */
  rv=EBC_Provider_XchgPubRequest(pro, sess, u, signVersion);
  if (rv) {
    DBG_ERROR(AQEBICS_LOGDOMAIN,
	      "Error exchanging PUB request (%d)", rv);
    if (doLock)
      AB_Banking_EndExclUseUser(ab, u, 1);
    GWEN_HttpSession_free(sess);
    return rv;
  }

  /* unlock user */
  if (doLock) {
    rv=AB_Banking_EndExclUseUser(ab, u, 0);
    if (rv<0) {
      DBG_ERROR(AQEBICS_LOGDOMAIN, "Could not unlock customer");
      AB_Banking_EndExclUseUser(ab, u, 1);
      GWEN_HttpSession_free(sess);
      return rv;
    }
  }

  /* close and destroy session */
  GWEN_HttpSession_Fini(sess);
  GWEN_HttpSession_free(sess);

  return rv;
}



int EBC_Provider_Download(AB_PROVIDER *pro, AB_USER *u,
                          const char *rtype,
			  GWEN_BUFFER *targetBuffer,
			  int withReceipt,
			  const GWEN_TIME *fromTime,
                          const GWEN_TIME *toTime,
                          int doLock) {
  EBC_PROVIDER *dp;
  GWEN_HTTP_SESSION *sess;
  int rv;
  EBC_USER_STATUS ust;
  AB_BANKING *ab;

  assert(pro);
  dp=GWEN_INHERIT_GETDATA(AB_PROVIDER, EBC_PROVIDER, pro);
  assert(dp);

  ab=AB_Provider_GetBanking(pro);

  ust=EBC_User_GetStatus(u);
  if (ust!=EBC_UserStatus_Enabled) {
    DBG_ERROR(AQEBICS_LOGDOMAIN,
	      "Invalid status \"%s\" of user \"%s\"",
	      EBC_User_Status_toString(ust),
	      AB_User_GetUserId(u));
    return GWEN_ERROR_INVALID;
  }

  /* create and open session */
  sess=EBC_Dialog_new(pro, u);
  rv=GWEN_HttpSession_Init(sess);
  if (rv<0) {
    DBG_ERROR(AQEBICS_LOGDOMAIN, "Could not open session");
    GWEN_HttpSession_free(sess);
    return rv;
  }

  /* lock user */
  if (doLock) {
    rv=AB_Banking_BeginExclUseUser(ab, u);
    if (rv<0) {
      DBG_ERROR(AQEBICS_LOGDOMAIN, "Could not lock customer");
      GWEN_HttpSession_free(sess);
      return rv;
    }
  }

  /* exchange request and response */
  rv=EBC_Provider_XchgDownloadRequest(pro, sess, u,
				      rtype, targetBuffer, withReceipt,
				      fromTime, toTime);
  if (rv) {
    DBG_ERROR(AQEBICS_LOGDOMAIN,
	      "Error exchanging download request (%d)", rv);
    if (doLock)
      AB_Banking_EndExclUseUser(ab, u, 1);
    GWEN_HttpSession_free(sess);
    return rv;
  }

  /* unlock user */
  if (doLock) {
    rv=AB_Banking_EndExclUseUser(ab, u, 0);
    if (rv<0) {
      DBG_ERROR(AQEBICS_LOGDOMAIN, "Could not unlock customer");
      AB_Banking_EndExclUseUser(ab, u, 1);
      GWEN_HttpSession_free(sess);
      return rv;
    }
  }

  /* close and destroy session */
  GWEN_HttpSession_Fini(sess);
  GWEN_HttpSession_free(sess);

  return rv;
}



int EBC_Provider_Upload(AB_PROVIDER *pro, AB_USER *u,
			const char *rtype,
			const uint8_t *pData,
                        uint32_t lData,
                        int doLock) {
  EBC_PROVIDER *dp;
  GWEN_HTTP_SESSION *sess;
  int rv;
  EBC_USER_STATUS ust;
  AB_BANKING *ab;

  assert(pro);
  dp=GWEN_INHERIT_GETDATA(AB_PROVIDER, EBC_PROVIDER, pro);
  assert(dp);

  ab=AB_Provider_GetBanking(pro);

  ust=EBC_User_GetStatus(u);
  if (ust!=EBC_UserStatus_Enabled) {
    DBG_ERROR(AQEBICS_LOGDOMAIN,
	      "Invalid status \"%s\" of user \"%s\"",
	      EBC_User_Status_toString(ust),
	      AB_User_GetUserId(u));
    return GWEN_ERROR_INVALID;
  }

  /* create and open session */
  sess=EBC_Dialog_new(pro, u);
  rv=GWEN_HttpSession_Init(sess);
  if (rv<0) {
    DBG_ERROR(AQEBICS_LOGDOMAIN, "Could not open session");
    GWEN_HttpSession_free(sess);
    return rv;
  }

  /* lock user */
  if (doLock) {
    rv=AB_Banking_BeginExclUseUser(ab, u);
    if (rv<0) {
      DBG_ERROR(AQEBICS_LOGDOMAIN, "Could not lock customer");
      GWEN_HttpSession_free(sess);
      return rv;
    }
  }

  /* exchange request and response */
  rv=EBC_Provider_XchgUploadRequest(pro, sess, u, rtype, pData, lData);
  if (rv) {
    DBG_ERROR(AQEBICS_LOGDOMAIN,
	      "Error exchanging upload request (%d)", rv);
    if (doLock)
      AB_Banking_EndExclUseUser(ab, u, 1);
    GWEN_HttpSession_free(sess);
    return rv;
  }

  /* unlock user */
  if (doLock) {
    rv=AB_Banking_EndExclUseUser(ab, u, 0);
    if (rv<0) {
      DBG_ERROR(AQEBICS_LOGDOMAIN, "Could not unlock customer");
      AB_Banking_EndExclUseUser(ab, u, 1);
      GWEN_HttpSession_free(sess);
      return rv;
    }
  }

  /* close and destroy session */
  GWEN_HttpSession_Fini(sess);
  GWEN_HttpSession_free(sess);

  return rv;
}



int EBC_Provider_CreateKeys(AB_PROVIDER *pro,
			    AB_USER *u,
			    int cryptAndAuthKeySizeInBytes,
			    int signKeySizeInBytes,
			    int nounmount) {
  EBC_PROVIDER *dp;
  GWEN_CRYPT_TOKEN *ct;
  const GWEN_CRYPT_TOKEN_CONTEXT *ctx;
  uint32_t keyId;
  GWEN_CRYPT_CRYPTALGO *algo;
  int rv;

  assert(pro);
  dp=GWEN_INHERIT_GETDATA(AB_PROVIDER, EBC_PROVIDER, pro);
  assert(dp);

  /* get token */
  rv=AB_Banking_GetCryptToken(AB_Provider_GetBanking(pro),
			      EBC_User_GetTokenType(u),
			      EBC_User_GetTokenName(u),
			      &ct);
  if (rv) {
    DBG_ERROR(AQEBICS_LOGDOMAIN,
	      "Error getting the user's crypt token (%d)", rv);
    return rv;
  }

  GWEN_Crypt_Token_AddModes(ct, GWEN_CRYPT_TOKEN_MODE_EXP_65537);

  /* create algo */
  algo=GWEN_Crypt_CryptAlgo_new(GWEN_Crypt_CryptAlgoId_Rsa,
				GWEN_Crypt_CryptMode_None);
  GWEN_Crypt_CryptAlgo_SetChunkSize(algo, cryptAndAuthKeySizeInBytes);

  /* open token for admin */
  if (!GWEN_Crypt_Token_IsOpen(ct)) {
    rv=GWEN_Crypt_Token_Open(ct, 1, 0);
    if (rv) {
      DBG_ERROR(AQEBICS_LOGDOMAIN,
		"Error opening crypt token (%d)", rv);
      GWEN_Crypt_CryptAlgo_free(algo);
      return rv;
    }
  }
  
  /* get context */
  ctx=GWEN_Crypt_Token_GetContext(ct, EBC_User_GetTokenContextId(u), 0);
  if (ctx==NULL) {
    DBG_ERROR(AQEBICS_LOGDOMAIN,
	      "Could not get context %d", EBC_User_GetTokenContextId(u));
    GWEN_Crypt_CryptAlgo_free(algo);
    return GWEN_ERROR_INVALID;
  }
  
  DBG_INFO(AQEBICS_LOGDOMAIN, "Creating keys, please wait...");
  
  /* get cipher key id */
  keyId=GWEN_Crypt_Token_Context_GetDecipherKeyId(ctx);
  if (keyId==0) {
    DBG_ERROR(AQEBICS_LOGDOMAIN,
	      "No decipher key id specified (internal error)");
    GWEN_Crypt_CryptAlgo_free(algo);
    return GWEN_ERROR_INVALID;
  }
  
  /* generate cipher key */
  rv=GWEN_Crypt_Token_GenerateKey(ct, keyId, algo, 0);
  if (rv) {
    DBG_ERROR(AQEBICS_LOGDOMAIN,
	      "Error generating key (%d)", rv);
    GWEN_Crypt_CryptAlgo_free(algo);
    return rv;
  }
  
  /* get auth sign key id */
  keyId=GWEN_Crypt_Token_Context_GetAuthSignKeyId(ctx);
  if (keyId) {
    /* generate auth sign key */
    rv=GWEN_Crypt_Token_GenerateKey(ct, keyId, algo, 0);
    if (rv) {
      DBG_ERROR(AQEBICS_LOGDOMAIN,
		"Error generating key (%d)", rv);
      GWEN_Crypt_CryptAlgo_free(algo);
      return rv;
    }
  }

  /* get sign key id */
  keyId=GWEN_Crypt_Token_Context_GetSignKeyId(ctx);
  if (keyId==0) {
    DBG_ERROR(AQEBICS_LOGDOMAIN,
	      "No sign key id specified (internal error)");
    GWEN_Crypt_CryptAlgo_free(algo);
    return GWEN_ERROR_INVALID;
  }
  
  /* generate sign key */
  GWEN_Crypt_CryptAlgo_SetChunkSize(algo, signKeySizeInBytes);
  rv=GWEN_Crypt_Token_GenerateKey(ct, keyId, algo, 0);
  if (rv) {
    DBG_ERROR(AQEBICS_LOGDOMAIN,
	      "Error generating key (%d)", rv);
    GWEN_Crypt_CryptAlgo_free(algo);
    return rv;
  }

  if (!nounmount) {
    /* close token */
    rv=GWEN_Crypt_Token_Close(ct, 0, 0);
    if (rv) {
      DBG_ERROR(AQEBICS_LOGDOMAIN,
		"Error closing crypt token (%d)", rv);
      GWEN_Crypt_CryptAlgo_free(algo);
      return rv;
    }
  }

  GWEN_Crypt_CryptAlgo_free(algo);
  return 0;
}



int EBC_Provider_CreateTempKey(AB_PROVIDER *pro,
			       AB_USER *u,
			       int signKeySizeInBytes,
			       int nounmount) {
  EBC_PROVIDER *dp;
  GWEN_CRYPT_TOKEN *ct;
  const GWEN_CRYPT_TOKEN_CONTEXT *ctx;
  uint32_t keyId;
  GWEN_CRYPT_CRYPTALGO *algo;
  int rv;

  assert(pro);
  dp=GWEN_INHERIT_GETDATA(AB_PROVIDER, EBC_PROVIDER, pro);
  assert(dp);

  /* get token */
  rv=AB_Banking_GetCryptToken(AB_Provider_GetBanking(pro),
			      EBC_User_GetTokenType(u),
			      EBC_User_GetTokenName(u),
			      &ct);
  if (rv) {
    DBG_ERROR(AQEBICS_LOGDOMAIN,
	      "Error getting the user's crypt token (%d)", rv);
    return rv;
  }

  GWEN_Crypt_Token_AddModes(ct, GWEN_CRYPT_TOKEN_MODE_EXP_65537);

  /* create algo */
  algo=GWEN_Crypt_CryptAlgo_new(GWEN_Crypt_CryptAlgoId_Rsa,
				GWEN_Crypt_CryptMode_None);

  /* open token for admin */
  if (!GWEN_Crypt_Token_IsOpen(ct)) {
    rv=GWEN_Crypt_Token_Open(ct, 1, 0);
    if (rv) {
      DBG_ERROR(AQEBICS_LOGDOMAIN,
		"Error opening crypt token (%d)", rv);
      GWEN_Crypt_CryptAlgo_free(algo);
      return rv;
    }
  }
  
  /* get context */
  ctx=GWEN_Crypt_Token_GetContext(ct, EBC_User_GetTokenContextId(u), 0);
  if (ctx==NULL) {
    DBG_ERROR(AQEBICS_LOGDOMAIN,
	      "Could not get context %d", EBC_User_GetTokenContextId(u));
    GWEN_Crypt_CryptAlgo_free(algo);
    return GWEN_ERROR_INVALID;
  }
  
  DBG_INFO(AQEBICS_LOGDOMAIN, "Creating keys, please wait...");
  
  /* get temp sign key id */
  keyId=GWEN_Crypt_Token_Context_GetTempSignKeyId(ctx);
  if (keyId==0) {
    DBG_ERROR(AQEBICS_LOGDOMAIN,
	      "No sign key id specified (internal error)");
    GWEN_Crypt_CryptAlgo_free(algo);
    return GWEN_ERROR_INVALID;
  }
  
  /* generate sign key */
  GWEN_Crypt_CryptAlgo_SetChunkSize(algo, signKeySizeInBytes);
  rv=GWEN_Crypt_Token_GenerateKey(ct, keyId, algo, 0);
  if (rv) {
    DBG_ERROR(AQEBICS_LOGDOMAIN,
	      "Error generating key (%d)", rv);
    GWEN_Crypt_CryptAlgo_free(algo);
    return rv;
  }

  if (!nounmount) {
    /* close token */
    rv=GWEN_Crypt_Token_Close(ct, 0, 0);
    if (rv) {
      DBG_ERROR(AQEBICS_LOGDOMAIN,
		"Error closing crypt token (%d)", rv);
      GWEN_Crypt_CryptAlgo_free(algo);
      return rv;
    }
  }

  GWEN_Crypt_CryptAlgo_free(algo);
  return 0;
}



int EBC_Provider_GetIniLetterTxt(AB_PROVIDER *pro,
				 AB_USER *u,
				 int useBankKey,
				 GWEN_BUFFER *lbuf,
				 int nounmount) {
  AB_BANKING *ab;
  const void *p;
  unsigned int l;
  GWEN_BUFFER *bbuf;
  int i;
  GWEN_TIME *ti;
  int rv;
  EBC_PROVIDER *dp;
  GWEN_CRYPT_TOKEN *ct;
  const GWEN_CRYPT_TOKEN_CONTEXT *ctx;
  uint32_t kid;
  const GWEN_CRYPT_TOKEN_KEYINFO *ki=NULL;
  const char *signVersion;

  assert(pro);
  dp=GWEN_INHERIT_GETDATA(AB_PROVIDER, EBC_PROVIDER, pro);
  assert(dp);

  assert(u);

  ab=AB_Provider_GetBanking(pro);
  assert(ab);

  signVersion=EBC_User_GetSignVersion(u);
  if (!(signVersion && *signVersion))
    signVersion="A005";

  /* get crypt token and context */
  rv=EBC_Provider_MountToken(pro, u, &ct, &ctx);
  if (rv<0) {
    DBG_INFO(AQEBICS_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }

  if (useBankKey) {
    /* get sign key info */
    kid=GWEN_Crypt_Token_Context_GetVerifyKeyId(ctx);
    if (kid) {
      ki=GWEN_Crypt_Token_GetKeyInfo(ct, kid,
				     GWEN_CRYPT_TOKEN_KEYFLAGS_HASMODULUS |
				     GWEN_CRYPT_TOKEN_KEYFLAGS_HASEXPONENT |
				     GWEN_CRYPT_TOKEN_KEYFLAGS_HASKEYVERSION |
				     GWEN_CRYPT_TOKEN_KEYFLAGS_HASKEYNUMBER,
				     0);
    }
    if (!ki ||
	!(GWEN_Crypt_Token_KeyInfo_GetFlags(ki) & GWEN_CRYPT_TOKEN_KEYFLAGS_HASMODULUS) |
	!(GWEN_Crypt_Token_KeyInfo_GetFlags(ki) & GWEN_CRYPT_TOKEN_KEYFLAGS_HASEXPONENT)) {
      kid=GWEN_Crypt_Token_Context_GetEncipherKeyId(ctx);
      if (kid) {
	ki=GWEN_Crypt_Token_GetKeyInfo(ct, kid,
				       GWEN_CRYPT_TOKEN_KEYFLAGS_HASMODULUS |
				       GWEN_CRYPT_TOKEN_KEYFLAGS_HASEXPONENT |
				       GWEN_CRYPT_TOKEN_KEYFLAGS_HASKEYVERSION |
				       GWEN_CRYPT_TOKEN_KEYFLAGS_HASKEYNUMBER,
				       0);
      }
    }
    if (!ki ||
	!(GWEN_Crypt_Token_KeyInfo_GetFlags(ki) & GWEN_CRYPT_TOKEN_KEYFLAGS_HASMODULUS) |
	!(GWEN_Crypt_Token_KeyInfo_GetFlags(ki) & GWEN_CRYPT_TOKEN_KEYFLAGS_HASEXPONENT)) {
      if (!nounmount)
	AB_Banking_ClearCryptTokenList(ab);
      DBG_ERROR(0, "Server keys missing, please get them first");
      GWEN_Gui_ProgressLog(0,
			   GWEN_LoggerLevel_Error,
			   I18N("Server keys missing, "
				"please get them first"));
      return GWEN_ERROR_NOT_FOUND;
    }
  }
  else {
    /* get sign key info */
    kid=GWEN_Crypt_Token_Context_GetSignKeyId(ctx);
    if (kid) {
      ki=GWEN_Crypt_Token_GetKeyInfo(ct, kid,
				     GWEN_CRYPT_TOKEN_KEYFLAGS_HASMODULUS |
				     GWEN_CRYPT_TOKEN_KEYFLAGS_HASEXPONENT |
				     GWEN_CRYPT_TOKEN_KEYFLAGS_HASKEYVERSION |
				     GWEN_CRYPT_TOKEN_KEYFLAGS_HASKEYNUMBER,
				     0);
    }
    if (!ki ||
	!(GWEN_Crypt_Token_KeyInfo_GetFlags(ki) & GWEN_CRYPT_TOKEN_KEYFLAGS_HASMODULUS) |
	!(GWEN_Crypt_Token_KeyInfo_GetFlags(ki) & GWEN_CRYPT_TOKEN_KEYFLAGS_HASEXPONENT)) {
      if (!nounmount)
	AB_Banking_ClearCryptTokenList(ab);
      DBG_ERROR(0, "User keys missing, please generate them first");
      GWEN_Gui_ProgressLog(0,
			   GWEN_LoggerLevel_Error,
			   I18N("User keys missing, "
				"please generate them first"));
      return GWEN_ERROR_NOT_FOUND;
    }
  }


  /* prelude */
  GWEN_Buffer_AppendString(lbuf, I18N("\n\n\nINI-Letter DFUE ("));

  GWEN_Buffer_AppendString(lbuf, signVersion);
  GWEN_Buffer_AppendString(lbuf, ")\n\n");
  GWEN_Buffer_AppendString(lbuf,
			   I18N("Date           : "));
  ti=GWEN_CurrentTime();
  assert(ti);
  GWEN_Time_toString(ti, I18N("YYYY/MM/DD"), lbuf);
  GWEN_Buffer_AppendString(lbuf, "\n");
  GWEN_Buffer_AppendString(lbuf,
			   I18N("Time           : "));
  GWEN_Time_toString(ti, I18N("hh:mm:ss"), lbuf);
  GWEN_Buffer_AppendString(lbuf, "\n");

  if (useBankKey) {
    GWEN_Buffer_AppendString(lbuf,
                             I18N("Bank Code      : "));
    GWEN_Buffer_AppendString(lbuf, AB_User_GetBankCode(u));
  }
  else {
    GWEN_Buffer_AppendString(lbuf,
                             I18N("User           : "));
    GWEN_Buffer_AppendString(lbuf, AB_User_GetUserId(u));
  }

  GWEN_Buffer_AppendString(lbuf, "\n");
  GWEN_Buffer_AppendString(lbuf,
                           I18N("Public key for electronic signature"));
  GWEN_Buffer_AppendString(lbuf, "\n\n");

  GWEN_Buffer_AppendString(lbuf, "  ");
  GWEN_Buffer_AppendString(lbuf,
                           I18N("Exponent"));
  GWEN_Buffer_AppendString(lbuf, "\n\n");

  /* exponent */
  p=GWEN_Crypt_Token_KeyInfo_GetExponentData(ki);
  l=GWEN_Crypt_Token_KeyInfo_GetExponentLen(ki);
  if (!p || !l) {
    DBG_ERROR(AQEBICS_LOGDOMAIN, "Bad key.");
    GWEN_Gui_ProgressLog(0, GWEN_LoggerLevel_Error,
                           I18N("Bad key"));
    return GWEN_ERROR_BAD_DATA;
  }

  bbuf=GWEN_Buffer_new(0, 129, 0, 1);
  GWEN_Buffer_AppendBytes(bbuf, p, l);
  GWEN_Buffer_Rewind(bbuf);
  if (l<128)
    GWEN_Buffer_FillLeftWithBytes(bbuf, 0, 128-l);
  p=GWEN_Buffer_GetStart(bbuf);
  l=GWEN_Buffer_GetUsedBytes(bbuf);
  for (i=0; i<8; i++) {
    GWEN_Buffer_AppendString(lbuf, "  ");
    if (GWEN_Text_ToHexBuffer(p, 16, lbuf, 2, ' ', 0)) {
      DBG_ERROR(0, "Error converting to hex??");
      abort();
    }
    p+=16;
    GWEN_Buffer_AppendString(lbuf, "\n");
  }
  GWEN_Buffer_free(bbuf);

  /* modulus */
  GWEN_Buffer_AppendString(lbuf, "\n");
  GWEN_Buffer_AppendString(lbuf, "  ");
  GWEN_Buffer_AppendString(lbuf,
                           I18N("Modulus"));
  GWEN_Buffer_AppendString(lbuf, "\n\n");
  p=GWEN_Crypt_Token_KeyInfo_GetModulusData(ki);
  l=GWEN_Crypt_Token_KeyInfo_GetModulusLen(ki);
  if (!p || !l) {
    DBG_ERROR(AQEBICS_LOGDOMAIN, "Bad key.");
    GWEN_Gui_ProgressLog(0, GWEN_LoggerLevel_Error,
                           I18N("Bad key"));
    return GWEN_ERROR_BAD_DATA;
  }

  bbuf=GWEN_Buffer_new(0, 129, 0, 1);
  GWEN_Buffer_AppendBytes(bbuf, p, l);
  GWEN_Buffer_Rewind(bbuf);
  if (l<128)
    GWEN_Buffer_FillLeftWithBytes(bbuf, 0, 128-l);
  p=GWEN_Buffer_GetStart(bbuf);
  l=GWEN_Buffer_GetUsedBytes(bbuf);
  for (i=0; i<8; i++) {
    GWEN_Buffer_AppendString(lbuf, "  ");
    if (GWEN_Text_ToHexBuffer(p, 16, lbuf, 2, ' ', 0)) {
      DBG_ERROR(0, "Error converting to hex??");
      abort();
    }
    p+=16;
    GWEN_Buffer_AppendString(lbuf, "\n");
  }
  GWEN_Buffer_free(bbuf);

  GWEN_Buffer_AppendString(lbuf, "\n");
  GWEN_Buffer_AppendString(lbuf, "  ");
  GWEN_Buffer_AppendString(lbuf,
                           I18N("Hash"));
  GWEN_Buffer_AppendString(lbuf, "\n\n");

  bbuf=GWEN_Buffer_new(0, 21, 0, 1);
  if (strcasecmp(signVersion, "A004")==0) {
    rv=EB_Key_Info_BuildSigHash_Rmd160(ki, bbuf);
    if (rv<0) {
      DBG_ERROR(AQEBICS_LOGDOMAIN, "Error hashing (%d)", rv);
      abort();
    }
    p=GWEN_Buffer_GetStart(bbuf);
    l=GWEN_Buffer_GetUsedBytes(bbuf);
    for (i=0; i<2; i++) {
      GWEN_Buffer_AppendString(lbuf, "  ");
      if (GWEN_Text_ToHexBuffer(p, 10, lbuf, 2, ' ', 0)) {
        DBG_ERROR(0, "Error converting to hex??");
        abort();
      }
      p+=10;
      GWEN_Buffer_AppendString(lbuf, "\n");
    }
  }
  else {
    rv=EB_Key_Info_BuildSigHash_Sha256(ki, bbuf);
    if (rv<0) {
      DBG_ERROR(AQEBICS_LOGDOMAIN, "Error hashing (%d)", rv);
      abort();
    }
    p=GWEN_Buffer_GetStart(bbuf);
    l=GWEN_Buffer_GetUsedBytes(bbuf);
    for (i=0; i<2; i++) {
      GWEN_Buffer_AppendString(lbuf, "  ");
      if (GWEN_Text_ToHexBuffer(p, 16, lbuf, 2, ' ', 0)) {
        DBG_ERROR(0, "Error converting to hex??");
        abort();
      }
      p+=16;
      GWEN_Buffer_AppendString(lbuf, "\n");
    }
  }

  GWEN_Buffer_free(bbuf);

  if (!useBankKey) {
    GWEN_Buffer_AppendString(lbuf, "\n\n");
    GWEN_Buffer_AppendString(lbuf,
			     I18N("I confirm that I created the above key "
				  "for my electronic signature.\n"));
    GWEN_Buffer_AppendString(lbuf, "\n\n");
    GWEN_Buffer_AppendString(lbuf,
			     I18N("____________________________  "
				  "____________________________\n"
				  "Place, date                   "
				  "Signature\n"));
  }

  return 0;
}



int EBC_Provider__addKiTxt(GWEN_UNUSED AB_PROVIDER *pro,
			   const GWEN_CRYPT_TOKEN_KEYINFO *ki,
                           GWEN_BUFFER *lbuf,
                           int version) {
  const uint8_t *p;
  unsigned int l;
  unsigned int nl;
  GWEN_BUFFER *bbuf;
  int i;
  int rv;

  GWEN_Buffer_AppendString(lbuf, "  ");
  GWEN_Buffer_AppendString(lbuf,
                           I18N("Exponent"));
  GWEN_Buffer_AppendString(lbuf, "\n\n");

  /* exponent */
  p=GWEN_Crypt_Token_KeyInfo_GetExponentData(ki);
  l=GWEN_Crypt_Token_KeyInfo_GetExponentLen(ki);
  if (!p || !l) {
    DBG_ERROR(AQEBICS_LOGDOMAIN, "Bad key.");
    GWEN_Gui_ProgressLog(0, GWEN_LoggerLevel_Error,
                           I18N("Bad key"));
    return GWEN_ERROR_BAD_DATA;
  }

  /* skip null bytes */
  while(*p==0 && l>1) {
    p++;
    l--;
  }

  /* fill to next multiple of 16 */
  nl=((l+15)/16)*16;
  bbuf=GWEN_Buffer_new(0, nl+1, 0, 1);
  if (l<nl)
    GWEN_Buffer_FillWithBytes(bbuf, 0, nl-l);
  GWEN_Buffer_AppendBytes(bbuf, (const char*)p, l);
  p=(const uint8_t*)GWEN_Buffer_GetStart(bbuf);
  l=GWEN_Buffer_GetUsedBytes(bbuf);

  for (i=0; i<(nl/16); i++) {
    GWEN_Buffer_AppendString(lbuf, "  ");
    if (GWEN_Text_ToHexBuffer((const char*)p, 16, lbuf, 2, ' ', 0)) {
      DBG_ERROR(0, "Error converting to hex??");
      abort();
    }
    p+=16;
    GWEN_Buffer_AppendString(lbuf, "\n");
  }
  GWEN_Buffer_free(bbuf);

  /* modulus */
  GWEN_Buffer_AppendString(lbuf, "\n");
  GWEN_Buffer_AppendString(lbuf, "  ");
  GWEN_Buffer_AppendString(lbuf,
                           I18N("Modulus"));
  GWEN_Buffer_AppendString(lbuf, "\n\n");
  p=GWEN_Crypt_Token_KeyInfo_GetModulusData(ki);
  l=GWEN_Crypt_Token_KeyInfo_GetModulusLen(ki);
  if (!p || !l) {
    DBG_ERROR(AQEBICS_LOGDOMAIN, "Bad key.");
    GWEN_Gui_ProgressLog(0, GWEN_LoggerLevel_Error,
                           I18N("Bad key"));
    return GWEN_ERROR_BAD_DATA;
  }

  nl=((l+15)/16)*16;
  bbuf=GWEN_Buffer_new(0, nl+1, 0, 1);
  if (l<nl)
    GWEN_Buffer_FillWithBytes(bbuf, 0, nl-l);
  GWEN_Buffer_AppendBytes(bbuf, (const char*)p, l);
  p=(const uint8_t*)GWEN_Buffer_GetStart(bbuf);
  l=GWEN_Buffer_GetUsedBytes(bbuf);
  for (i=0; i<(nl/16); i++) {
    GWEN_Buffer_AppendString(lbuf, "  ");
    if (GWEN_Text_ToHexBuffer((const char*)p, 16, lbuf, 2, ' ', 0)) {
      DBG_ERROR(0, "Error converting to hex??");
      abort();
    }
    p+=16;
    GWEN_Buffer_AppendString(lbuf, "\n");
  }
  GWEN_Buffer_free(bbuf);

  GWEN_Buffer_AppendString(lbuf, "\n");
  GWEN_Buffer_AppendString(lbuf, "  ");
  GWEN_Buffer_AppendString(lbuf,
                           I18N("Hash"));
  GWEN_Buffer_AppendString(lbuf, "\n\n");

  bbuf=GWEN_Buffer_new(0, 21, 0, 1);
  switch(version) {
  case 1:
    rv=EB_Key_Info_BuildHashSha1(ki, bbuf, 0);
    if (rv<0) {
      DBG_ERROR(AQEBICS_LOGDOMAIN, "Error hashing (%d)", rv);
      abort();
    }

    p=(const uint8_t*)GWEN_Buffer_GetStart(bbuf);
    l=GWEN_Buffer_GetUsedBytes(bbuf);
    for (i=0; i<2; i++) {
      GWEN_Buffer_AppendString(lbuf, "  ");
      if (GWEN_Text_ToHexBuffer((const char*)p, 10, lbuf, 2, ' ', 0)) {
        DBG_ERROR(0, "Error converting to hex??");
        abort();
      }
      p+=10;
      GWEN_Buffer_AppendString(lbuf, "\n");
    }
    break;

  case 2:
  default:
    rv=EB_Key_Info_BuildSigHash_Sha256(ki, bbuf);
    if (rv<0) {
      DBG_ERROR(AQEBICS_LOGDOMAIN, "Error hashing (%d)", rv);
      abort();
    }
    p=(const uint8_t*)GWEN_Buffer_GetStart(bbuf);
    l=GWEN_Buffer_GetUsedBytes(bbuf);
    for (i=0; i<2; i++) {
      GWEN_Buffer_AppendString(lbuf, "  ");
      if (GWEN_Text_ToHexBuffer((const char*)p, 16, lbuf, 2, ' ', 0)) {
        DBG_ERROR(0, "Error converting to hex??");
        abort();
      }
      p+=16;
      GWEN_Buffer_AppendString(lbuf, "\n");
    }
  }
  GWEN_Buffer_free(bbuf);

  return 0;
}



int EBC_Provider_GetHiaLetterTxt(AB_PROVIDER *pro,
				 AB_USER *u,
				 int useBankKey,
				 GWEN_BUFFER *lbuf,
				 int nounmount) {
  AB_BANKING *ab;
  GWEN_TIME *ti;
  int rv;
  EBC_PROVIDER *dp;
  GWEN_CRYPT_TOKEN *ct;
  const GWEN_CRYPT_TOKEN_CONTEXT *ctx;
  uint32_t kid;
  const GWEN_CRYPT_TOKEN_KEYINFO *ki=NULL;
  const char *authVersion;
  const char *cryptVersion;
  int i;

  assert(pro);
  dp=GWEN_INHERIT_GETDATA(AB_PROVIDER, EBC_PROVIDER, pro);
  assert(dp);

  assert(u);

  ab=AB_Provider_GetBanking(pro);
  assert(ab);

  cryptVersion=EBC_User_GetCryptVersion(u);
  if (!(cryptVersion && *cryptVersion))
    cryptVersion="E002";
  authVersion=EBC_User_GetAuthVersion(u);
  if (!(authVersion && *authVersion))
    authVersion="X002";

  /* get crypt token and context */
  rv=EBC_Provider_MountToken(pro, u, &ct, &ctx);
  if (rv<0) {
    DBG_INFO(AQEBICS_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }

  /* handle auth key */
  if (useBankKey) {
    /* get auth key info */
    kid=GWEN_Crypt_Token_Context_GetAuthVerifyKeyId(ctx);
    if (kid) {
      ki=GWEN_Crypt_Token_GetKeyInfo(ct, kid,
				     GWEN_CRYPT_TOKEN_KEYFLAGS_HASMODULUS |
				     GWEN_CRYPT_TOKEN_KEYFLAGS_HASEXPONENT |
				     GWEN_CRYPT_TOKEN_KEYFLAGS_HASKEYVERSION |
				     GWEN_CRYPT_TOKEN_KEYFLAGS_HASKEYNUMBER,
				     0);
    }
    if (!ki ||
	!(GWEN_Crypt_Token_KeyInfo_GetFlags(ki) & GWEN_CRYPT_TOKEN_KEYFLAGS_HASMODULUS) |
	!(GWEN_Crypt_Token_KeyInfo_GetFlags(ki) & GWEN_CRYPT_TOKEN_KEYFLAGS_HASEXPONENT)) {
      if (!nounmount)
	AB_Banking_ClearCryptTokenList(ab);
      DBG_ERROR(0, "Server keys missing, please get them first");
      GWEN_Gui_ProgressLog(0,
			   GWEN_LoggerLevel_Error,
			   I18N("Server keys missing, "
				"please get them first"));
      return GWEN_ERROR_NOT_FOUND;
    }
  }
  else {
    /* get sign key info */
    kid=GWEN_Crypt_Token_Context_GetAuthSignKeyId(ctx);
    if (kid) {
      ki=GWEN_Crypt_Token_GetKeyInfo(ct, kid,
				     GWEN_CRYPT_TOKEN_KEYFLAGS_HASMODULUS |
				     GWEN_CRYPT_TOKEN_KEYFLAGS_HASEXPONENT |
				     GWEN_CRYPT_TOKEN_KEYFLAGS_HASKEYVERSION |
				     GWEN_CRYPT_TOKEN_KEYFLAGS_HASKEYNUMBER,
				     0);
    }
    if (!ki ||
	!(GWEN_Crypt_Token_KeyInfo_GetFlags(ki) & GWEN_CRYPT_TOKEN_KEYFLAGS_HASMODULUS) |
	!(GWEN_Crypt_Token_KeyInfo_GetFlags(ki) & GWEN_CRYPT_TOKEN_KEYFLAGS_HASEXPONENT)) {
      if (!nounmount)
	AB_Banking_ClearCryptTokenList(ab);
      DBG_ERROR(0, "User keys missing, please generate them first");
      GWEN_Gui_ProgressLog(0,
			   GWEN_LoggerLevel_Error,
			   I18N("User keys missing, "
				"please generate them first"));
      return GWEN_ERROR_NOT_FOUND;
    }
  }

  /* prelude */
  GWEN_Buffer_AppendString(lbuf,
			   I18N("\n\n\nINI-Letter HIA\n\n"));
  GWEN_Buffer_AppendString(lbuf,
			   I18N("Date           : "));
  ti=GWEN_CurrentTime();
  assert(ti);
  GWEN_Time_toString(ti, I18N("YYYY/MM/DD"), lbuf);
  GWEN_Buffer_AppendString(lbuf, "\n");
  GWEN_Buffer_AppendString(lbuf,
			   I18N("Time           : "));
  GWEN_Time_toString(ti, I18N("hh:mm:ss"), lbuf);
  GWEN_Buffer_AppendString(lbuf, "\n");

  if (useBankKey) {
    GWEN_Buffer_AppendString(lbuf,
                             I18N("Bank Code      : "));
    GWEN_Buffer_AppendString(lbuf, AB_User_GetBankCode(u));
  }
  else {
    GWEN_Buffer_AppendString(lbuf,
                             I18N("User           : "));
    GWEN_Buffer_AppendString(lbuf, AB_User_GetUserId(u));
  }
  GWEN_Buffer_AppendString(lbuf, "\n\n");

  /* add auth key */
  GWEN_Buffer_AppendString(lbuf, I18N("Public key for authentication signature ("));
  GWEN_Buffer_AppendString(lbuf, authVersion);
  GWEN_Buffer_AppendString(lbuf, ")\n\n");

  if (strcasecmp(authVersion, "X001")==0)
    i=1;
  else
    i=2;
  rv=EBC_Provider__addKiTxt(pro, ki, lbuf, i);
  if (rv<0) {
    DBG_INFO(AQEBICS_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }

  /* handle crypt key */
  ki=NULL;
  if (useBankKey) {
    /* get encipher key info */
    kid=GWEN_Crypt_Token_Context_GetEncipherKeyId(ctx);
    if (kid) {
      ki=GWEN_Crypt_Token_GetKeyInfo(ct, kid,
				     GWEN_CRYPT_TOKEN_KEYFLAGS_HASMODULUS |
				     GWEN_CRYPT_TOKEN_KEYFLAGS_HASEXPONENT |
				     GWEN_CRYPT_TOKEN_KEYFLAGS_HASKEYVERSION |
				     GWEN_CRYPT_TOKEN_KEYFLAGS_HASKEYNUMBER,
				     0);
    }
    if (!ki ||
	!(GWEN_Crypt_Token_KeyInfo_GetFlags(ki) & GWEN_CRYPT_TOKEN_KEYFLAGS_HASMODULUS) |
	!(GWEN_Crypt_Token_KeyInfo_GetFlags(ki) & GWEN_CRYPT_TOKEN_KEYFLAGS_HASEXPONENT)) {
      if (!nounmount)
	AB_Banking_ClearCryptTokenList(ab);
      DBG_ERROR(0, "Server keys missing, please get them first");
      GWEN_Gui_ProgressLog(0,
			   GWEN_LoggerLevel_Error,
			   I18N("Server keys missing, "
				"please get them first"));
      return GWEN_ERROR_NOT_FOUND;
    }
  }
  else {
    /* get decipher key info */
    kid=GWEN_Crypt_Token_Context_GetDecipherKeyId(ctx);
    if (kid) {
      ki=GWEN_Crypt_Token_GetKeyInfo(ct, kid,
				     GWEN_CRYPT_TOKEN_KEYFLAGS_HASMODULUS |
				     GWEN_CRYPT_TOKEN_KEYFLAGS_HASEXPONENT |
				     GWEN_CRYPT_TOKEN_KEYFLAGS_HASKEYVERSION |
				     GWEN_CRYPT_TOKEN_KEYFLAGS_HASKEYNUMBER,
				     0);
    }
    if (!ki ||
	!(GWEN_Crypt_Token_KeyInfo_GetFlags(ki) & GWEN_CRYPT_TOKEN_KEYFLAGS_HASMODULUS) |
	!(GWEN_Crypt_Token_KeyInfo_GetFlags(ki) & GWEN_CRYPT_TOKEN_KEYFLAGS_HASEXPONENT)) {
      if (!nounmount)
	AB_Banking_ClearCryptTokenList(ab);
      DBG_ERROR(0, "User keys missing, please generate them first");
      GWEN_Gui_ProgressLog(0,
			   GWEN_LoggerLevel_Error,
			   I18N("User keys missing, "
				"please generate them first"));
      return GWEN_ERROR_NOT_FOUND;
    }
  }

  /* add crypt key */
  GWEN_Buffer_AppendString(lbuf, "\n\n");
  GWEN_Buffer_AppendString(lbuf,
                           I18N("Public key for encryption ("));
  GWEN_Buffer_AppendString(lbuf, cryptVersion);
  GWEN_Buffer_AppendString(lbuf, ")\n\n");

  if (strcasecmp(cryptVersion, "E001")==0)
    i=1;
  else
    i=2;
  rv=EBC_Provider__addKiTxt(pro, ki, lbuf, i);
  if (rv<0) {
    DBG_INFO(AQEBICS_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }

  if (!useBankKey) {
    GWEN_Buffer_AppendString(lbuf, "\n\n");
    GWEN_Buffer_AppendString(lbuf,
			     I18N("I confirm that I created the above "
				  "keys.\n"));
    GWEN_Buffer_AppendString(lbuf, "\n\n");
    GWEN_Buffer_AppendString(lbuf,
			     I18N("____________________________  "
				  "____________________________\n"
				  "Place, date                   "
				  "Signature\n"));
  }

  return 0;
}



int EBC_Provider_Sha256(const uint8_t *pData, uint32_t lData, GWEN_BUFFER *hbuf) {
  GWEN_BUFFER *tbuf;
  GWEN_MDIGEST *md;
  int rv;

  tbuf=GWEN_Buffer_new(0, lData, 0, 1);
  while(lData--) {
    uint8_t c;

    c=*(pData++);
    if (c!=13 && c!=10 && c!=26)
      GWEN_Buffer_AppendByte(tbuf, c);
  }

  /* hash (RMD160) */
  md=GWEN_MDigest_Sha256_new();
  rv=GWEN_MDigest_Begin(md);
  if (rv<0) {
    DBG_INFO(AQEBICS_LOGDOMAIN, "here (%d)", rv);
    GWEN_MDigest_free(md);
    GWEN_Buffer_free(tbuf);
    return rv;
  }
  rv=GWEN_MDigest_Update(md,
			 (const uint8_t*)GWEN_Buffer_GetStart(tbuf),
			 GWEN_Buffer_GetUsedBytes(tbuf));
  if (rv<0) {
    DBG_INFO(AQEBICS_LOGDOMAIN, "here (%d)", rv);
    GWEN_MDigest_free(md);
    GWEN_Buffer_free(tbuf);
    return rv;
  }
  rv=GWEN_MDigest_End(md);
  if (rv<0) {
    DBG_INFO(AQEBICS_LOGDOMAIN, "here (%d)", rv);
    GWEN_MDigest_free(md);
    GWEN_Buffer_free(tbuf);
    return rv;
  }
  GWEN_Buffer_AppendBytes(hbuf,
			  (const char*)GWEN_MDigest_GetDigestPtr(md),
			  GWEN_MDigest_GetDigestSize(md));
  GWEN_MDigest_free(md);
  GWEN_Buffer_free(tbuf);

  return 0;
}



