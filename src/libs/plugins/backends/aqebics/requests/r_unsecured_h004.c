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


#include "r_unsecured_l.h"

#include "aqebics/aqebics_l.h"
#include "aqebics/msg/msg.h"
#include "aqebics/msg/keys.h"
#include "aqebics/msg/zip.h"
#include "aqebics/msg/xml.h"
#include "aqebics/client/user_l.h"

#include <gwenhywfar/base64.h>
#include <gwenhywfar/gui.h>
#include <gwenhywfar/httpsession.h>




EB_MSG *EBC_Provider_MkUnsecuredRequest_H004(AB_PROVIDER *pro,
                                             AB_USER *u,
                                             const char *orderType,
                                             const char *orderAttribute,
                                             const char *orderData)
{
  EB_MSG *msg;
  xmlDocPtr doc;
  xmlNodePtr root_node = NULL;
  xmlNodePtr node = NULL;
  xmlNsPtr ns;
  const char *s;
  GWEN_BUFFER *tbuf;
  int rv;

  /* create request */
  msg=EB_Msg_new();
  doc=EB_Msg_GetDoc(msg);
  root_node=xmlNewNode(NULL, BAD_CAST "ebicsUnsecuredRequest");
  xmlDocSetRootElement(doc, root_node);
  ns=xmlNewNs(root_node,
              BAD_CAST "http://www.ebics.org/H004",
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
               BAD_CAST "urn:org:ebics:H004 ebics_ keymgmt_request_H004.xsd");
  xmlNewProp(root_node, BAD_CAST "Version", BAD_CAST "H004");
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
  EB_Msg_SetCharValue(msg, "header/static/UserID", AB_User_GetUserId(u));
  EB_Msg_SetCharValue(msg, "header/static/OrderDetails/OrderType", orderType);

  tbuf=GWEN_Buffer_new(0, 16, 0, 1);
  rv=EBC_Provider_Generate_OrderId(pro, tbuf);
  if (rv<0) {
    DBG_ERROR(AQEBICS_LOGDOMAIN, "Error exchanging messages (%d)", rv);
    GWEN_Buffer_free(tbuf);
    EB_Msg_free(msg);
    return NULL;
  }
  EB_Msg_SetCharValue(msg, "header/static/OrderDetails/OrderID", GWEN_Buffer_GetStart(tbuf));
  GWEN_Buffer_free(tbuf);

  EB_Msg_SetCharValue(msg,
                      "header/static/OrderDetails/OrderAttribute",
                      orderAttribute);
  EB_Msg_SetCharValue(msg, "header/static/SecurityMedium", "0200");
  EB_Msg_SetCharValue(msg, "body/DataTransfer/OrderData", orderData);

  return msg;
}




