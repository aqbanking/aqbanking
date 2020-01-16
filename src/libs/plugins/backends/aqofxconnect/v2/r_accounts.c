/***************************************************************************
 begin       : Mon Jan 13 2020
 copyright   : (C) 2020 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif


#include "n_header.h"
#include "n_signon.h"
#include "n_acctinfo.h"





int AO_Provider_V2_RequestAccounts(AB_PROVIDER *pro, AB_USER *u, int keepOpen)
{
  GWEN_XMLNODE *xmlRoot;
  GWEN_XMLNODE *xmlNode;
  GWEN_BUFFER *bufRequest;
  GWEN_BUFFER *bufResponse=NULL;
  int rv;

  /* prepare XML request */
  xmlRoot=GWEN_XMLNode_new(GWEN_XMLNodeTypeTag, "root");
  xmlNode=AO_V2_MkOfxHeader(u);
  if (xmlNode)
    GWEN_XMLNode_AddHeader(xmlRoot, xmlNode);

  xmlNode=AO_V2_MkSignOnNode(u);
  if (xmlNode)
    GWEN_XMLNode_AddChild(xmlRoot, xmlNode);

  xmlNode=AO_V2_MkAcctInfoRqNode(u);
  if (xmlNode)
    GWEN_XMLNode_AddChild(xmlRoot, xmlNode);

  bufRequest=GWEN_Buffer_new(0, 256, 0, 1);
  rv=GWEN_XMLNode_toBuffer(xmlRoot, bufRequest, GWEN_XML_FLAGS_HANDLE_HEADERS | GWEN_XML_FLAGS_SIMPLE);
  if (rv<0) {
    DBG_INFO(AQOFXCONNECT_LOGDOMAIN, "here (%d)", rv);
    GWEN_Buffer_free(bufRequest);
    GWEN_XMLNode_free(xmlRoot);
    return rv;
  }
  GWEN_XMLNode_free(xmlRoot);

  /* exchange messages */
  rv=AO_Provider_SendAndReceive(pro, u,
				(const uint8_t*) GWEN_Buffer_GetStart(bufRequest),
				GWEN_Buffer_GetUsedBytes(bufRequest),
				&bufResponse);
  if (rv<0) {
    DBG_INFO(AQOFXCONNECT_LOGDOMAIN, "here (%d)", rv);
    GWEN_Buffer_free(bufRequest);
    return rv;
  }
  GWEN_Buffer_free(bufRequest);

  /* parse response */
  xmlRoot=GWEN_XMLNode_fromString(GWEN_Buffer_GetStart(bufResponse),
				  GWEN_Buffer_GetUsedBytes(bufResponse),
				  GWEN_XML_FLAGS_HANDLE_HEADERS | GWEN_XML_FLAGS_SIMPLE);
  if (xmlRoot==NULL) {
    DBG_INFO(AQOFXCONNECT_LOGDOMAIN, "Bad data in OFX response:");
    GWEN_Text_LogString(GWEN_Buffer_GetStart(bufResponse),
			GWEN_Buffer_GetUsedBytes(bufResponse),
			AQOFXCONNECT_LOGDOMAIN,
			GWEN_LoggerLevel_Error);
    GWEN_Buffer_free(bufResponse);
    return rv;
  }
  GWEN_Buffer_free(bufResponse);


  /* TODO: import data from XML */


  GWEN_XMLNode_free(xmlRoot);
  return 0;
}



