

#include "msg/msg.h"
#include "msg/keys.h"
#include "msg/zip.h"
#include "user_l.h"

#include <gwenhywfar/base64.h>



static int EBC_Provider_XchgIniRequest_H003(AB_PROVIDER *pro,
					    GWEN_HTTP_SESSION *sess,
					    AB_USER *u) {
  EBC_PROVIDER *dp;
  int rv;
  GWEN_CRYPT_TOKEN *ct;
  const GWEN_CRYPT_TOKEN_CONTEXT *ctx;
  uint32_t kid;
  const GWEN_CRYPT_TOKEN_KEYINFO *signKeyInfo=NULL;
  xmlNsPtr ns;
  EB_MSG *msg;
  const char *userId;
  const char *partnerId;
  EB_MSG *mRsp;
  EB_RC rc;
  xmlDocPtr doc;
  xmlNodePtr root_node = NULL;
  xmlNodePtr node = NULL;
  GWEN_BUFFER *tbuf;
  const char *signVersion;
  const char *s;
  GWEN_BUFFER *bufB64;

  assert(pro);
  dp=GWEN_INHERIT_GETDATA(AB_PROVIDER, EBC_PROVIDER, pro);
  assert(dp);

  userId=AB_User_GetUserId(u);
  partnerId=AB_User_GetCustomerId(u);

  /* get crypt token and context */
  rv=EBC_Provider_MountToken(pro, u, &ct, &ctx);
  if (rv<0) {
    DBG_INFO(AQEBICS_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }

  /* get crypt key info */
  kid=GWEN_Crypt_Token_Context_GetSignKeyId(ctx);
  if (kid) {
    signKeyInfo=GWEN_Crypt_Token_GetKeyInfo(ct, kid,
					    GWEN_CRYPT_TOKEN_KEYFLAGS_HASMODULUS |
					    GWEN_CRYPT_TOKEN_KEYFLAGS_HASEXPONENT |
					    GWEN_CRYPT_TOKEN_KEYFLAGS_HASKEYVERSION |
					    GWEN_CRYPT_TOKEN_KEYFLAGS_HASKEYNUMBER,
					    0);
    if (signKeyInfo==NULL) {
      DBG_ERROR(AQEBICS_LOGDOMAIN, "Sign key info not found on crypt token");
      GWEN_Gui_ProgressLog(0,
			   GWEN_LoggerLevel_Error,
			   I18N("Sign key info not found on crypt token"));
      return GWEN_ERROR_NOT_FOUND;
    }
  }

  signVersion=EBC_User_GetSignVersion(u);
  if (! (signVersion && *signVersion))
    signVersion="A005";

  if (strcasecmp(signVersion, "A005")==0) {
    /* create INIRequestOrderData */
    doc=xmlNewDoc(BAD_CAST "1.0");
    doc->encoding=xmlCharStrdup("UTF-8");
    root_node=xmlNewNode(NULL, BAD_CAST "SignaturePubKeyOrderData");
    xmlDocSetRootElement(doc, root_node);
    ns=xmlNewNs(root_node,
		BAD_CAST "http://www.ebics.org/S001",
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
		 BAD_CAST "http://www.ebics.org/S001 "
		 "http://www.ebics.org/S001/ebics_signature.xsd");
  
    /* create auth key tree */
    node=xmlNewChild(root_node, NULL,
		     BAD_CAST "SignaturePubKeyInfo", NULL);
    rv=EB_Key_Info_toXml(signKeyInfo, node);
    if (rv<0) {
      DBG_ERROR(AQEBICS_LOGDOMAIN, "Error response: (%d)", rv);
      xmlFreeDoc(doc);
      return GWEN_ERROR_INVALID;
    }
    xmlNewChild(node, NULL,
                BAD_CAST "SignatureVersion",
                BAD_CAST signVersion);
  
    /* store partner id and user id */
    node=xmlNewChild(root_node, NULL,
		     BAD_CAST "PartnerID",
		     BAD_CAST partnerId);
  
    node=xmlNewChild(root_node, NULL,
		     BAD_CAST "UserID",
		     BAD_CAST userId);

#if 0
    DBG_ERROR(0, "Sending this key data:");
    xmlDocDump(stderr, doc);
#endif

    /* compress and base64 doc */
    bufB64=GWEN_Buffer_new(0, 4096, 0, 1);
    rv=EB_Xml_Compress64Doc(doc, bufB64);
    if (rv<0) {
      DBG_ERROR(AQEBICS_LOGDOMAIN, "Error compressing/encoding doc (%d)", rv);
      xmlFreeDoc(doc);
      return rv;
    }
    xmlFreeDoc(doc);
  }
  else {
    DBG_ERROR(AQEBICS_LOGDOMAIN, "Unsupported sign version [%s]", signVersion);
    return GWEN_ERROR_INTERNAL;
  }

  /* create request */
  msg=EB_Msg_new();
  doc=EB_Msg_GetDoc(msg);
  root_node=xmlNewNode(NULL, BAD_CAST "ebicsUnsecuredRequest");
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

  /* body */
  node=xmlNewChild(root_node, NULL, BAD_CAST "body", NULL);

  /* fill */
  s=EBC_User_GetPeerId(u);
  if (s)
    EB_Msg_SetCharValue(msg, "header/static/HostID", s);
  s=AB_User_GetCustomerId(u);
  if (s)
    EB_Msg_SetCharValue(msg, "header/static/PartnerID", s);
  EB_Msg_SetCharValue(msg, "header/static/UserID",
		      AB_User_GetUserId(u));
  EB_Msg_SetCharValue(msg, "header/static/OrderDetails/OrderType", "INI");
  tbuf=GWEN_Buffer_new(0, 16, 0, 1);
  rv=EBC_Provider_Generate_OrderId(pro, tbuf);
  if (rv<0) {
    DBG_ERROR(AQEBICS_LOGDOMAIN, "Error creating order id (%d)", rv);
    GWEN_Buffer_free(tbuf);
    GWEN_Buffer_free(bufB64);
    EB_Msg_free(msg);
    return rv;
  }
  EB_Msg_SetCharValue(msg, "header/static/OrderDetails/OrderID",
		     GWEN_Buffer_GetStart(tbuf));
  GWEN_Buffer_free(tbuf);
  EB_Msg_SetCharValue(msg,
		      "header/static/OrderDetails/OrderAttribute",
		      "DZNNN");
  EB_Msg_SetCharValue(msg, "header/static/SecurityMedium", "0000");
  EB_Msg_SetCharValue(msg, "body/DataTransfer/OrderData",
		      GWEN_Buffer_GetStart(bufB64));
  GWEN_Buffer_free(bufB64);

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

  EB_Msg_free(mRsp);

  /* adjust user status and flags */
  DBG_NOTICE(AQEBICS_LOGDOMAIN, "Adjusting user flags");
  EBC_User_AddFlags(u, EBC_USER_FLAGS_INI);
  if ((EBC_User_GetFlags(u) & (EBC_USER_FLAGS_INI | EBC_USER_FLAGS_HIA))
      ==
      (EBC_USER_FLAGS_INI | EBC_USER_FLAGS_HIA))
    EBC_User_SetStatus(u, EBC_UserStatus_Init2);
  else
    EBC_User_SetStatus(u, EBC_UserStatus_Init1);

  return 0;
}







