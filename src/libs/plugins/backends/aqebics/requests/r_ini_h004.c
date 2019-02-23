/***************************************************************************
    begin       : Mon Mar 01 2004
    copyright   : (C) 2019 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif


#include "r_ini_l.h"

#include "aqebics_l.h"
#include "msg/msg.h"
#include "msg/keys.h"
#include "msg/zip.h"
#include "msg/xml.h"
#include "user_l.h"
#include "provider_l.h"
#include "r_unsecured_l.h"

#include <gwenhywfar/base64.h>
#include <gwenhywfar/gui.h>
#include <gwenhywfar/httpsession.h>




static int _mkOrderData_INI_H004(AB_PROVIDER *pro, AB_USER *u, GWEN_BUFFER *bufB64);




int EBC_Provider_XchgIniRequest_H004(AB_PROVIDER *pro, GWEN_HTTP_SESSION *sess, AB_USER *u)
{
  int rv;
  EB_MSG *msg;
  EB_MSG *mRsp;
  GWEN_BUFFER *bufB64;

  /* create order data */
  bufB64=GWEN_Buffer_new(0, 4096, 0, 1);

  rv=_mkOrderData_INI_H004(pro, u, bufB64);
  if (rv<0) {
    DBG_INFO(AQEBICS_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }

  /* create request */
  msg=EBC_Provider_MkUnsecuredRequest_H004(pro, u, "INI", "DZNNN", GWEN_Buffer_GetStart(bufB64));
  if (msg==NULL) {
    DBG_INFO(AQEBICS_LOGDOMAIN, "here");
    GWEN_Buffer_free(bufB64);
    return GWEN_ERROR_GENERIC;
  }
  GWEN_Buffer_free(bufB64);

  /* exchange requests */
  rv=EBC_Dialog_ExchangeMessagesAndCheckResponse(sess, msg, &mRsp);
  if (rv<0 || rv>=300) {
    DBG_ERROR(AQEBICS_LOGDOMAIN, "Error exchanging messages (%d)", rv);
    EB_Msg_free(msg);
    return rv;
  }
  EB_Msg_free(msg);

  /* log results */
  EBC_Provider_LogRequestResults(pro, mRsp, NULL);

  /* nothing else to do with response */
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



int _mkOrderData_INI_H004(AB_PROVIDER *pro, AB_USER *u, GWEN_BUFFER *bufB64)
{
  int rv;
  GWEN_CRYPT_TOKEN *ct;
  const GWEN_CRYPT_TOKEN_CONTEXT *ctx;
  uint32_t kid;
  const GWEN_CRYPT_TOKEN_KEYINFO *signKeyInfo=NULL;
  xmlNsPtr ns;
  const char *userId;
  const char *partnerId;
  const char *signVersion;

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
  if (!(signVersion && *signVersion))
    signVersion="A005";

  if (strcasecmp(signVersion, "A005")==0) {
    xmlDocPtr doc;
    xmlNodePtr root_node = NULL;
    xmlNodePtr node = NULL;

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

  return 0;
}







