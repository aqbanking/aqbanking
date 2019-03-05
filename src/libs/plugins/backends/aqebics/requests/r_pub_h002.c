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


#include "r_pub_l.h"

#include "aqebics/aqebics_l.h"
#include "aqebics/msg/msg.h"
#include "aqebics/msg/keys.h"
#include "aqebics/msg/zip.h"
#include "aqebics/msg/xml.h"
#include "aqebics/client/user_l.h"
#include "aqebics/client/provider_l.h"
#include "aqebics/requests/r_upload_l.h"

#include <gwenhywfar/base64.h>
#include <gwenhywfar/gui.h>
#include <gwenhywfar/httpsession.h>




int EBC_Provider_XchgPubRequest_H002(AB_PROVIDER *pro,
                                     GWEN_HTTP_SESSION *sess,
                                     AB_USER *u,
                                     const char *signVersion)
{
  int rv;
  const char *userId;
  const char *partnerId;
  GWEN_BUFFER *bufKey;
  GWEN_CRYPT_TOKEN *ct;
  const GWEN_CRYPT_TOKEN_CONTEXT *ctx;
  uint32_t kid;
  const GWEN_CRYPT_TOKEN_KEYINFO *signKeyInfo=NULL;

  userId=AB_User_GetUserId(u);
  partnerId=AB_User_GetCustomerId(u);

  /* get crypt token and context */
  rv=EBC_Provider_MountToken(pro, u, &ct, &ctx);
  if (rv<0) {
    DBG_INFO(AQEBICS_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }

  /* get crypt key info */
  kid=GWEN_Crypt_Token_Context_GetTempSignKeyId(ctx);
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

  if (strcasecmp(signVersion, "A004")==0) {
    EB_RC rc;

    /* encode according to "DFUE-Abkommen" */
    bufKey=GWEN_Buffer_new(0, 512, 0, 1);
    rc=EB_Key_Info_toBin(signKeyInfo, userId, "A004", 1024, bufKey);
    if (rc) {
      DBG_ERROR(AQEBICS_LOGDOMAIN, "Error writing key (rc=%06x)", rc);
      GWEN_Buffer_free(bufKey);
      return GWEN_ERROR_GENERIC;
    }
  }
  else {
    xmlDocPtr doc;
    xmlNodePtr root_node = NULL;
    xmlNodePtr node = NULL;
    xmlNsPtr ns;

    /* create INIRequestOrderData */
    doc=xmlNewDoc(BAD_CAST "1.0");
    doc->encoding=xmlCharStrdup("UTF-8");
    root_node=xmlNewNode(NULL, BAD_CAST "PUBRequestOrderData");
    xmlDocSetRootElement(doc, root_node);
    ns=xmlNewNs(root_node,
                BAD_CAST "http://www.ebics.org/H002",
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
                 BAD_CAST "http://www.ebics.org/H002 "
                 "http://www.ebics.org/H002/ebics_orders.xsd");

    /* create sign key tree */
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

    /* compress and base64 doc */
    bufKey=GWEN_Buffer_new(0, 4096, 0, 1);
    rv=EB_Xml_Compress64Doc(doc, bufKey);
    if (rv<0) {
      DBG_ERROR(AQEBICS_LOGDOMAIN, "Error compressing/encoding doc (%d)", rv);
      xmlFreeDoc(doc);
      return rv;
    }
    xmlFreeDoc(doc);
  }

  rv=EBC_Provider_XchgUploadRequest(pro, sess, u, "PUB",
                                    (const uint8_t *)GWEN_Buffer_GetStart(bufKey),
                                    GWEN_Buffer_GetUsedBytes(bufKey));
  if (rv<0) {
    DBG_INFO(AQEBICS_LOGDOMAIN, "here (%d)", rv);
    GWEN_Buffer_free(bufKey);
    return rv;
  }

  GWEN_Buffer_free(bufKey);
  return 0;
}




