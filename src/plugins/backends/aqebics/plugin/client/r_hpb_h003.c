

#include "msg/msg.h"
#include "msg/keys.h"
#include "msg/zip.h"
#include "msg/xml.h"
#include "user_l.h"

#include <gwenhywfar/base64.h>



static int EBC_Provider_XchgHpbRequest_H003(AB_PROVIDER *pro,
					    GWEN_HTTP_SESSION *sess,
					    AB_USER *u) {
  EBC_PROVIDER *dp;
  int rv;
  GWEN_CRYPT_TOKEN *ct;
  const GWEN_CRYPT_TOKEN_CONTEXT *ctx;
  uint32_t keyId;
  xmlNsPtr ns;
  EB_MSG *msg;
  EB_MSG *mRsp;
  EB_RC rc;
  xmlDocPtr doc;
  xmlNodePtr root_node = NULL;
  xmlNodePtr node = NULL;
  xmlNodePtr sigNode = NULL;
  GWEN_BUFFER *tbuf;
  const char *s;

  assert(pro);
  dp=GWEN_INHERIT_GETDATA(AB_PROVIDER, EBC_PROVIDER, pro);
  assert(dp);

  /* get crypt token and context */
  rv=EBC_Provider_MountToken(pro, u, &ct, &ctx);
  if (rv<0) {
    DBG_INFO(AQEBICS_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }

  /* create request */
  msg=EB_Msg_new();
  doc=EB_Msg_GetDoc(msg);
  root_node=xmlNewNode(NULL, BAD_CAST "ebicsNoPubKeyDigestsRequest");
  xmlDocSetRootElement(doc, root_node);
  ns=xmlNewNs(root_node,
	      BAD_CAST "http://www.ebics.org/H003",
	      NULL);
  assert(ns);
  ns=xmlNewNs(root_node,
	      BAD_CAST "http://www.w3.org/2000/09/xmldsig#",
	      BAD_CAST "ds");
  assert(ns);
  ns=xmlNewNs(root_node,
	      BAD_CAST "http://www.w3.org/2001/XMLSchema-instance",
	      BAD_CAST "xsi");
  xmlNewNsProp(root_node,
	       ns,
	       BAD_CAST "schemaLocation", /* xsi:schemaLocation */
	       BAD_CAST "http://www.ebics.org/H003 "
	       "http://www.ebics.org/H003/ebics_keymgmt_request.xsd");

  xmlNewProp(root_node, BAD_CAST "Version", BAD_CAST "H003");
  xmlNewProp(root_node, BAD_CAST "Revision", BAD_CAST "1");

  /* header */
  node=xmlNewChild(root_node, NULL, BAD_CAST "header", NULL);
  xmlNewProp(node, BAD_CAST "authenticate", BAD_CAST "true");
  xmlNewChild(node, NULL, BAD_CAST "static", NULL);
  xmlNewChild(node, NULL, BAD_CAST "mutable", NULL);

  sigNode=xmlNewChild(root_node, NULL, BAD_CAST "AuthSignature", NULL);

  /* body */
  node=xmlNewChild(root_node, NULL, BAD_CAST "body", NULL);

  /* fill */
  s=EBC_User_GetPeerId(u);
  if (s)
    EB_Msg_SetCharValue(msg, "header/static/HostID", s);

  /* generate Nonce */
  tbuf=GWEN_Buffer_new(0, 128, 0, 1);
  rv=EBC_Provider_GenerateNonce(pro, tbuf);
  if (rv<0) {
    DBG_INFO(AQEBICS_LOGDOMAIN, "here (%d)", rv);
    GWEN_Buffer_free(tbuf);
    EB_Msg_free(msg);
    return rv;
  }
  EB_Msg_SetCharValue(msg, "header/static/Nonce", GWEN_Buffer_GetStart(tbuf));
  GWEN_Buffer_Reset(tbuf);

  /* generate timestamp */
  rv=EBC_Provider_GenerateTimeStamp(pro, u, tbuf);
  if (rv<0) {
    DBG_INFO(AQEBICS_LOGDOMAIN, "here (%d)", rv);
    GWEN_Buffer_free(tbuf);
    EB_Msg_free(msg);
    return rv;
  }
  EB_Msg_SetCharValue(msg, "header/static/Timestamp", GWEN_Buffer_GetStart(tbuf));
  GWEN_Buffer_free(tbuf);

  s=AB_User_GetCustomerId(u);
  if (s)
    EB_Msg_SetCharValue(msg, "header/static/PartnerID", s);
  EB_Msg_SetCharValue(msg, "header/static/UserID",
		      AB_User_GetUserId(u));
  EB_Msg_SetCharValue(msg, "header/static/OrderDetails/OrderType", "HPB");

  EB_Msg_SetCharValue(msg,
		      "header/static/OrderDetails/OrderAttribute",
		      "DZHNN");
  EB_Msg_SetCharValue(msg, "header/static/SecurityMedium", "0000");

  /* sign */
  rv=EBC_Provider_SignMessage(pro, msg, u, sigNode);
  if (rv<0) {
    DBG_INFO(AQEBICS_LOGDOMAIN, "here (%d)", rv);
    EB_Msg_free(msg);
    return rv;
  }

  /* exchange requests */
  rv=EBC_Dialog_ExchangeMessages(sess, msg, &mRsp);
  if (rv<0 || rv>=300) {
    DBG_ERROR(AQEBICS_LOGDOMAIN, "Error exchanging messages (%d)", rv);
    EB_Msg_free(msg);
    return rv;
  }
  EB_Msg_free(msg);

  /* check response */
  assert(mRsp);

  /* log results */
  EBC_Provider_LogRequestResults(pro, mRsp, NULL);

  rc=EB_Msg_GetResultCode(mRsp);
  if ((rc & 0xff0000)==0x090000 ||
      (rc & 0xff0000)==0x060000) {
    DBG_ERROR(AQEBICS_LOGDOMAIN, "Error response: (%06x)", rc);
    EB_Msg_free(mRsp);
    return AB_ERROR_SECURITY;
  }
  rc=EB_Msg_GetBodyResultCode(mRsp);
  if (rc) {
    if ((rc & 0xff0000)==0x090000 ||
	(rc & 0xff0000)==0x060000) {
      DBG_ERROR(AQEBICS_LOGDOMAIN, "Error response: (%06x)", rc);
      EB_Msg_free(mRsp);
      if ((rc & 0xfff00)==0x091300 ||
	  (rc & 0xfff00)==0x091200)
	return AB_ERROR_SECURITY;
      else
	return GWEN_ERROR_GENERIC;
    }
  }
  if (1) {
    xmlDocPtr orderDoc=NULL;
    xmlNodePtr root_node=NULL;
    xmlNodePtr node=NULL;
    GWEN_CRYPT_KEY *skey=NULL;
    GWEN_BUFFER *buf1;
    GWEN_BUFFER *buf2;
    const char *s;

    /* extract keys and store them */
    node=EB_Xml_GetNode(EB_Msg_GetRootNode(mRsp),
			"body/DataTransfer/DataEncryptionInfo",
			GWEN_PATH_FLAGS_NAMEMUSTEXIST);
    if (node==NULL) {
      DBG_ERROR(AQEBICS_LOGDOMAIN,
		"Bad message from server: Missing session key");
      EB_Msg_free(mRsp);
      return GWEN_ERROR_BAD_DATA;
    }
    rv=EBC_Provider_ExtractSessionKey(pro, u, node, &skey);
    if (rv<0) {
      DBG_INFO(AQEBICS_LOGDOMAIN, "here (%d)", rv);
      EB_Msg_free(mRsp);
      return rv;
    }

    s=EB_Msg_GetCharValue(mRsp, "body/DataTransfer/OrderData", NULL);
    if (!s) {
      DBG_ERROR(AQEBICS_LOGDOMAIN,
		"Bad message from server: Missing OrderData");
      EB_Msg_free(mRsp);
      return GWEN_ERROR_BAD_DATA;
    }
    buf1=GWEN_Buffer_new(0, strlen(s), 0, 1);
    rv=GWEN_Base64_Decode((const uint8_t*)s, 0, buf1);
    if (rv<0) {
      DBG_INFO(AQEBICS_LOGDOMAIN, "Could not decode OrderData (%d)", rv);
      GWEN_Buffer_free(buf1);
      EB_Msg_free(mRsp);
      return rv;
    }

    /* decode data */
    buf2=GWEN_Buffer_new(0, GWEN_Buffer_GetUsedBytes(buf1), 0, 1);
    rv=EBC_Provider_DecryptData(pro, u, skey,
				(const uint8_t*)GWEN_Buffer_GetStart(buf1),
				GWEN_Buffer_GetUsedBytes(buf1),
				buf2);
    if (rv<0) {
      DBG_INFO(AQEBICS_LOGDOMAIN, "Could not decrypt OrderData (%d)", rv);
      GWEN_Buffer_free(buf2);
      GWEN_Buffer_free(buf1);
      return rv;
    }

    /* parse XML document */
    rv=EB_Xml_DocFromBuffer(GWEN_Buffer_GetStart(buf2),
			    GWEN_Buffer_GetUsedBytes(buf2),
			    &orderDoc);
    GWEN_Buffer_free(buf2);
    GWEN_Buffer_free(buf1);
    if (rv<0) {
      DBG_INFO(AQEBICS_LOGDOMAIN, "here (%d)", rv);
      EB_Msg_free(mRsp);
      return rv;
    }

    /* get keys */
    root_node=xmlDocGetRootElement(orderDoc);

    /* get auth key */
    node=EB_Xml_GetNode(root_node, "AuthenticationPubKeyInfo",
			GWEN_PATH_FLAGS_NAMEMUSTEXIST);
    if (node==NULL) {
      DBG_ERROR(AQEBICS_LOGDOMAIN,
		"No authentication key found");
      xmlFreeDoc(orderDoc);
      EB_Msg_free(mRsp);
      return GWEN_ERROR_BAD_DATA;
    }
    else {
      const GWEN_CRYPT_TOKEN_KEYINFO *cki;
      GWEN_CRYPT_TOKEN_KEYINFO *ki;

      keyId=GWEN_Crypt_Token_Context_GetAuthVerifyKeyId(ctx);

      cki=GWEN_Crypt_Token_GetKeyInfo(ct, keyId, 0, 0);
      if (cki)
	ki=GWEN_Crypt_Token_KeyInfo_dup(cki);
      else
	ki=GWEN_Crypt_Token_KeyInfo_new(keyId,
					GWEN_Crypt_CryptAlgoId_Rsa,
					128);
      GWEN_Crypt_Token_KeyInfo_SetFlags(ki, 0);
      rc=EB_Key_Info_ReadXml(ki, node);
      if (rc) {
	DBG_INFO(AQEBICS_LOGDOMAIN, "here (%06x)", rc);
        GWEN_Crypt_Token_KeyInfo_free(ki);
	xmlFreeDoc(orderDoc);
	EB_Msg_free(mRsp);
	return GWEN_ERROR_BAD_DATA;
      }

      rv=GWEN_Crypt_Token_SetKeyInfo(ct, keyId, ki, 0);
      GWEN_Crypt_Token_KeyInfo_free(ki);
      if (rv) {
	DBG_INFO(AQEBICS_LOGDOMAIN, "here (%d)", rv);
	xmlFreeDoc(orderDoc);
	EB_Msg_free(mRsp);
	return rv;
      }

      DBG_NOTICE(AQEBICS_LOGDOMAIN, "Auth key stored");
    }

    /* get crypt key */
    node=EB_Xml_GetNode(root_node, "EncryptionPubKeyInfo",
			GWEN_PATH_FLAGS_NAMEMUSTEXIST);
    if (node==NULL) {
      DBG_ERROR(AQEBICS_LOGDOMAIN,
		"No encryption key found");
      xmlFreeDoc(orderDoc);
      EB_Msg_free(mRsp);
      return GWEN_ERROR_BAD_DATA;
    }
    else {
      const GWEN_CRYPT_TOKEN_KEYINFO *cki;
      GWEN_CRYPT_TOKEN_KEYINFO *ki;

      keyId=GWEN_Crypt_Token_Context_GetEncipherKeyId(ctx);

      cki=GWEN_Crypt_Token_GetKeyInfo(ct, keyId, 0, 0);
      if (cki)
	ki=GWEN_Crypt_Token_KeyInfo_dup(cki);
      else
	ki=GWEN_Crypt_Token_KeyInfo_new(keyId,
					GWEN_Crypt_CryptAlgoId_Rsa,
					128);
      GWEN_Crypt_Token_KeyInfo_SetFlags(ki, 0);
      rc=EB_Key_Info_ReadXml(ki, node);
      if (rc) {
	DBG_INFO(AQEBICS_LOGDOMAIN, "here (%06x)", rc);
        GWEN_Crypt_Token_KeyInfo_free(ki);
	xmlFreeDoc(orderDoc);
	EB_Msg_free(mRsp);
	return GWEN_ERROR_BAD_DATA;
      }

      rv=GWEN_Crypt_Token_SetKeyInfo(ct, keyId, ki, 0);
      GWEN_Crypt_Token_KeyInfo_free(ki);
      if (rv) {
	DBG_INFO(AQEBICS_LOGDOMAIN, "here (%d)", rv);
	xmlFreeDoc(orderDoc);
	EB_Msg_free(mRsp);
	return rv;
      }

      DBG_NOTICE(AQEBICS_LOGDOMAIN, "Crypt key stored");
    }

    xmlFreeDoc(orderDoc);
  }

  EB_Msg_free(mRsp);

  /* adjust user status and flags */
  DBG_NOTICE(AQEBICS_LOGDOMAIN, "Adjusting user flags");
  if ((EBC_User_GetFlags(u) &
       (EBC_USER_FLAGS_INI | EBC_USER_FLAGS_HIA))
      ==
      (EBC_USER_FLAGS_INI | EBC_USER_FLAGS_HIA))
    EBC_User_SetStatus(u, EBC_UserStatus_Enabled);

  return 0;
}







