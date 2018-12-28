/***************************************************************************
    begin       : Mon Mar 01 2004
    copyright   : (C) 2004-2010 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "msg_p.h"
#include "xml.h"

#include <xmlsec/transforms.h>
#include <xmlsec/errors.h>

#include <libxml/xpathInternals.h>

#include <gwenhywfar/debug.h>
#include <gwenhywfar/misc.h>
#include <gwenhywfar/mdigest.h>
#include <gwenhywfar/base64.h>


GWEN_INHERIT_FUNCTIONS(EB_MSG)



void EB_Msg__initWithDoc(EB_MSG *m) {
  xmlNodePtr rootNode;
  const char *s;

  assert(m);
  m->xpathCtx=xmlXPathNewContext(m->doc);

  if (xmlXPathRegisterNs(m->xpathCtx,
                         BAD_CAST "ds",
                         BAD_CAST "http://www.w3.org/2000/09/xmldsig#")!= 0) {
    DBG_ERROR(AQEBICS_LOGDOMAIN, "Unable to register NS");
    abort();
  }

  if (xmlXPathRegisterNs(m->xpathCtx,
                         BAD_CAST "xsi",
                         BAD_CAST "http://www.w3.org/2001/XMLSchema-instance")!= 0) {
    DBG_ERROR(AQEBICS_LOGDOMAIN, "Unable to register NS");
    abort();
  }

  if (m->hVersion==NULL) {
    rootNode=xmlDocGetRootElement(m->doc);
    s=(const char*)xmlGetProp(rootNode, BAD_CAST "Version");
    if (!(s && *s))
      s="H000";
    free(m->hVersion);
    m->hVersion=strdup(s);
  }
}



EB_MSG *EB_Msg_new() {
  EB_MSG *m;

  GWEN_NEW_OBJECT(EB_MSG, m);
  GWEN_INHERIT_INIT(EB_MSG, m);
  m->usage=1;
  m->doc=xmlNewDoc(BAD_CAST "1.0");
  m->doc->encoding=xmlCharStrdup("UTF-8");
  EB_Msg__initWithDoc(m);

  return m;
}



EB_MSG *EB_Msg_fromBuffer(const char *buffer, int size) {
  EB_MSG *m;

  GWEN_NEW_OBJECT(EB_MSG, m);
  GWEN_INHERIT_INIT(EB_MSG, m);
  m->usage=1;
  m->doc=xmlParseMemory(buffer, size);
  if (m->doc==0) {
    DBG_ERROR(AQEBICS_LOGDOMAIN, "Unable to parser buffer as XML doc");
    EB_Msg_free(m);
    return 0;
  }
  EB_Msg__initWithDoc(m);

  return m;
}



EB_MSG *EB_Msg_fromFile(const char *fname) {
  EB_MSG *m;

  GWEN_NEW_OBJECT(EB_MSG, m);
  GWEN_INHERIT_INIT(EB_MSG, m);
  m->usage=1;
  m->doc=xmlParseFile(fname);
  if (m->doc==0) {
    DBG_ERROR(AQEBICS_LOGDOMAIN, "Unable to parser file \"%s\" as XML doc",
              fname);
    EB_Msg_free(m);
    return 0;
  }
  EB_Msg__initWithDoc(m);

  return m;
}



EB_MSG *EB_Msg_newResponse(int willSign, const char *rName, const char *hVersion) {
  EB_MSG *m;

  GWEN_NEW_OBJECT(EB_MSG, m);
  GWEN_INHERIT_INIT(EB_MSG, m);
  m->usage=1;
  m->doc=EB_Msg__generateResponse(willSign, rName, hVersion);
  assert(m->doc);
  EB_Msg__initWithDoc(m);

  return m;
}



EB_MSG *EB_Msg_newRequest(int willSign, const char *hVersion) {
  EB_MSG *m;

  GWEN_NEW_OBJECT(EB_MSG, m);
  GWEN_INHERIT_INIT(EB_MSG, m);
  m->usage=1;
  m->doc=EB_Msg__generateRequest(willSign, hVersion);
  assert(m->doc);
  EB_Msg__initWithDoc(m);

  return m;
}



void EB_Msg_toBuffer(EB_MSG *m, GWEN_BUFFER *buf) {
  xmlChar *xmlbuff;
  int buffersize;

  assert(m);
  assert(m->usage);
  xmlDocDumpFormatMemory(m->doc, &xmlbuff, &buffersize, 0);
  GWEN_Buffer_AppendBytes(buf, (const char*)xmlbuff, (uint32_t)buffersize);
  xmlFree(xmlbuff);
}



void EB_Msg_free(EB_MSG *m) {
  if (m) {
    assert(m->usage);
    if (m->usage==1) {
      GWEN_INHERIT_FINI(EB_MSG, m);
      m->usage=0;
      free(m->hVersion);
      xmlFreeDoc(m->doc);
      GWEN_FREE_OBJECT(m);
    }
    else
      m->usage--;
  }
}



#if 0
xmlNodeSetPtr EB_Xml_GetNodes(EB_MSG *m, const char *xpathExpr) {
  xmlNodeSetPtr nodes;
  xmlXPathObjectPtr xpathObj;

  assert(m);
  assert(m->usage);
  xpathObj=xmlXPathEvalExpression(BAD_CAST xpathExpr, m->xpathCtx);
  if(xpathObj == NULL) {
    DBG_ERROR(AQEBICS_LOGDOMAIN,
              "Unable to evaluate xpath expression \"%s\"",
              xpathExpr);
    return 0;
  }

  nodes=xpathObj->nodesetval;
  if (!nodes) {
    DBG_INFO(AQEBICS_LOGDOMAIN, "No matching nodes");
    xmlXPathFreeObject(xpathObj);
    return 0;
  }
  xpathObj->nodesetval=NULL;

  xmlXPathFreeObject(xpathObj);
  return nodes;
}
#endif



xmlDocPtr EB_Msg_GetDoc(const EB_MSG *m) {
  assert(m);
  assert(m->usage);
  return m->doc;
}



xmlNodePtr EB_Msg_GetRootNode(EB_MSG *m) {
  assert(m);
  assert(m->usage);
  assert(m->doc);
  return xmlDocGetRootElement(m->doc);
}



const char *EB_Msg_GetHVersion(const EB_MSG *m) {
  assert(m);
  assert(m->usage);
  return m->hVersion;
}



void EB_Msg_SetHVersion(EB_MSG *m, const char *s) {
  assert(m);
  assert(m->usage);
  free(m->hVersion);
  if (s) m->hVersion=strdup(s);
  else m->hVersion=NULL;
}



int EB_Msg_BuildHashData(EB_MSG *m, GWEN_BUFFER *hbuf) {
  int rv;

  assert(m);
  assert(m->usage);

  rv=EB_Xml_BuildHashData(xmlDocGetRootElement(m->doc),
			  BAD_CAST "#xpointer(//*[@authenticate='true'])",
			  hbuf);
  if (rv) {
    DBG_INFO(AQEBICS_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }

  return 0;
}



int EB_Msg_BuildHashSha1(EB_MSG *m, GWEN_BUFFER *hbuf) {
  return EB_Xml_BuildNodeHashSha1(xmlDocGetRootElement(m->doc),
				  "#xpointer(//*[@authenticate='true'])",
				  hbuf);
}



int EB_Msg_BuildHashSha256(EB_MSG *m, GWEN_BUFFER *hbuf) {
  return EB_Xml_BuildNodeHashSha256(xmlDocGetRootElement(m->doc),
				    "#xpointer(//*[@authenticate='true'])",
				    hbuf);
}



int EB_Msg_BuildHashSha256Sha256(EB_MSG *m, GWEN_BUFFER *hbuf) {
  return EB_Xml_BuildNodeHashSha256Sha256(xmlDocGetRootElement(m->doc),
					  "#xpointer(//*[@authenticate='true'])",
					  hbuf);
}



int EB_Msg_ReadHash(EB_MSG *m, GWEN_BUFFER *hbuf) {
  const char *s;
  int rv;

  assert(m);
  assert(m->usage);

  s=EB_Xml_GetCharValue(xmlDocGetRootElement(m->doc),
                        "AuthSignature/"
                        "ds:SignedInfo/ds:Reference/ds:DigestValue",
                        0);
  if (!s) {
    DBG_ERROR(AQEBICS_LOGDOMAIN, "No hash");
    return -1;
  }

  rv=GWEN_Base64_Decode((const unsigned char*)s, 0, hbuf);
  if (rv) {
    DBG_ERROR(AQEBICS_LOGDOMAIN, "Could not decode hash");
    return -1;
  }

  return 0;
}



int EB_Msg_WriteHash(EB_MSG *m, const unsigned char *hash, int hsize) {
  int rv;
  GWEN_BUFFER *hbuf;

  assert(m);
  assert(m->usage);

  if (hsize!=20) {
    DBG_ERROR(AQEBICS_LOGDOMAIN, "Bad hash size (expected 20, was %d)",
              hsize);
    return -1;
  }

  hbuf=GWEN_Buffer_new(0, 40, 0, 1);
  rv=GWEN_Base64_Encode(hash, (uint32_t) hsize, hbuf, 0);
  if (rv) {
    DBG_ERROR(AQEBICS_LOGDOMAIN, "Could not base64-encode hash (%d)", rv);
    GWEN_Buffer_free(hbuf);
    return -1;
  }
  EB_Xml_SetCharValue(xmlDocGetRootElement(m->doc),
                      "AuthSignature/"
                      "ds:SignedInfo/ds:Reference/ds:DigestValue",
                      GWEN_Buffer_GetStart(hbuf));
  GWEN_Buffer_free(hbuf);

  return 0;
}



int EB_Msg_ReadSignature(EB_MSG *m, GWEN_BUFFER *hbuf) {
  const char *s;
  int rv;

  assert(m);
  assert(m->usage);

  s=EB_Xml_GetCharValue(xmlDocGetRootElement(m->doc),
                        "AuthSignature/"
                        "ds:SignatureValue",
                        0);
  if (!s) {
    DBG_ERROR(AQEBICS_LOGDOMAIN, "No hash");
    return -1;
  }

  rv=GWEN_Base64_Decode((const unsigned char*)s, 0, hbuf);
  if (rv) {
    DBG_ERROR(AQEBICS_LOGDOMAIN, "Could not decode signature");
    return -1;
  }

  return 0;
}



int EB_Msg_WriteSignature(EB_MSG *m, const unsigned char *hash, int hsize) {
  int rv;
  GWEN_BUFFER *hbuf;

  assert(m);
  assert(m->usage);

  if (hsize!=128) {
    DBG_ERROR(AQEBICS_LOGDOMAIN,
	      "Bad signature size (expected 128, was %d)",
	      hsize);
    return -1;
  }

  hbuf=GWEN_Buffer_new(0, 256, 0, 1);
  rv=GWEN_Base64_Encode(hash, (uint32_t) hsize, hbuf, 0);
  if (rv) {
    DBG_ERROR(AQEBICS_LOGDOMAIN,
	      "Could not base64-encode signature (%d)", rv);
    GWEN_Buffer_free(hbuf);
    return -1;
  }
  EB_Xml_SetCharValue(xmlDocGetRootElement(m->doc),
                      "AuthSignature/"
		      "ds:SignatureValue",
		      GWEN_Buffer_GetStart(hbuf));
  GWEN_Buffer_free(hbuf);

  return 0;
}



int EB_Msg_SetCharValue(EB_MSG *m, const char *path, const char *value) {
  assert(m);
  assert(m->usage);

  return EB_Xml_SetCharValue(xmlDocGetRootElement(m->doc), path, value);
}



const char *EB_Msg_GetCharValue(const EB_MSG *m, const char *path,
                                const char *defValue) {
  assert(m);
  assert(m->usage);

  return EB_Xml_GetCharValue(xmlDocGetRootElement(m->doc),
                             path, defValue);
}



int EB_Msg_SetIntValue(EB_MSG *m, const char *path, int value) {
  assert(m);
  assert(m->usage);

  return EB_Xml_SetIntValue(xmlDocGetRootElement(m->doc), path, value);
}



int EB_Msg_GetIntValue(const EB_MSG *m, const char *path, int defValue) {
  assert(m);
  assert(m->usage);

  return EB_Xml_GetIntValue(xmlDocGetRootElement(m->doc),
			    path, defValue);
}



EB_RC EB_Msg_GetResultCode(const EB_MSG *m) {
  const char *s;

  s=EB_Msg_GetCharValue(m, "header/mutable/ReturnCode", 0);
  if (s) {
    long unsigned int i;

    sscanf(s, "%lx", &i);
    return (EB_RC) i;
  }
  return EB_RC_INTERNAL_ERROR;
}



EB_RC EB_Msg_GetBodyResultCode(const EB_MSG *m) {
  const char *s;

  s=EB_Msg_GetCharValue(m, "body/ReturnCode", 0);
  if (s) {
    long unsigned int i;

    sscanf(s, "%lx", &i);
    return (EB_RC) i;
  }
  return EB_RC_INTERNAL_ERROR;
}







xmlDocPtr EB_Msg__generateRequest(int willSign, const char *hVersion) {
  xmlDocPtr doc=NULL;
  xmlNodePtr root_node = NULL;
  xmlNodePtr node = NULL;
  xmlNodePtr nodeX = NULL;
  xmlNodePtr nodeXX = NULL;

  /*
   * Creates a new document, a node and set it as a root node
   */
  doc=xmlNewDoc(BAD_CAST "1.0");
  root_node=xmlNewNode(NULL, BAD_CAST "ebics");
  xmlDocSetRootElement(doc, root_node);
  EB_Xml_Ebicsify(root_node, hVersion);

  node=xmlNewChild(root_node, NULL, BAD_CAST "header", NULL);
  xmlNewProp(node, BAD_CAST "authenticate", BAD_CAST "true");

  nodeX=xmlNewChild(node, NULL, BAD_CAST "static", NULL);
  xmlNewChild(nodeX, NULL, BAD_CAST "PartnerID", NULL);
  xmlNewChild(nodeX, NULL, BAD_CAST "UserID", NULL);
  nodeXX=xmlNewChild(nodeX, NULL, BAD_CAST "OrderDetails", NULL);
  xmlNewChild(nodeXX, NULL, BAD_CAST "OrderType", NULL);
  xmlNewChild(nodeXX, NULL, BAD_CAST "OrderID", NULL);
  xmlNewChild(nodeXX, NULL, BAD_CAST "OrderAttribute", NULL);

  nodeX=xmlNewChild(node, NULL, BAD_CAST "mutable", NULL);
  xmlNewChild(nodeX, NULL, BAD_CAST "TransactionPhase", NULL);

  if (willSign)
    EB_Msg__prepareSignature(doc);

  node=xmlNewChild(root_node, NULL, BAD_CAST "body", NULL);

  return doc;
}



xmlDocPtr EB_Msg__generateResponse(int willSign, const char *rName, const char *hVersion) {
  xmlDocPtr doc=NULL;
  xmlNodePtr root_node = NULL;
  xmlNodePtr node = NULL;
  xmlNodePtr nodeX = NULL;

  /*
   * Creates a new document, a node and set it as a root node
   */
  doc=xmlNewDoc(BAD_CAST "1.0");
  root_node=xmlNewNode(NULL, BAD_CAST rName);
  xmlDocSetRootElement(doc, root_node);
  EB_Xml_Ebicsify(root_node, hVersion);

  node=xmlNewChild(root_node, NULL, BAD_CAST "header", NULL);
  xmlNewProp(node, BAD_CAST "authenticate", BAD_CAST "true");

  nodeX=xmlNewChild(node, NULL, BAD_CAST "static", NULL);

  nodeX=xmlNewChild(node, NULL, BAD_CAST "mutable", NULL);
  //if (!isKeyMgt)
  xmlNewChild(nodeX, NULL, BAD_CAST "TransactionPhase", NULL);
  xmlNewChild(nodeX, NULL, BAD_CAST "ReturnCode", NULL);
  xmlNewChild(nodeX, NULL, BAD_CAST "ReportText", NULL);

  if (willSign)
    EB_Msg__prepareSignature(doc);

  node=xmlNewChild(root_node, NULL, BAD_CAST "body", NULL);

  return doc;
}



int EB_Msg__prepareSignature(xmlDocPtr doc) {
  xmlNodePtr node;
  xmlNsPtr ns;
  xmlNodePtr n;
  xmlNodePtr nn;
  xmlNodePtr nnn;
  xmlNodePtr nnnn;

  node=xmlNewChild(xmlDocGetRootElement(doc),
                   NULL, BAD_CAST "AuthSignature", NULL);
  ns=xmlSearchNs(doc, node, BAD_CAST "ds");
  assert(ns);

  n=xmlNewChild(node, ns, BAD_CAST "SignedInfo", NULL);
  nn=xmlNewChild(n, ns, BAD_CAST "CanonicalizationMethod", NULL);
  xmlNewProp(nn,
             BAD_CAST "Algorithm",
             BAD_CAST "http://www.w3.org/TR/2001/REC-xml-c14n-20010315");

  nn=xmlNewChild(n, ns, BAD_CAST "SignatureMethod", NULL);
  xmlNewProp(nn,
             BAD_CAST "Algorithm",
             BAD_CAST "http://www.w3.org/2000/09/xmldsig#rsa-sha1");

  nn=xmlNewChild(n, ns, BAD_CAST "Reference", NULL);
  xmlNewProp(nn,
             BAD_CAST "URI",
             BAD_CAST "#xpointer(//*[@authenticate='true'])");

  nnn=xmlNewChild(nn, ns, BAD_CAST "Transforms", NULL);
  nnnn=xmlNewChild(nnn, ns, BAD_CAST "Transform", NULL);
  xmlNewProp(nnnn,
             BAD_CAST "Algorithm",
             BAD_CAST "http://www.w3.org/TR/2001/REC-xml-c14n-20010315");

  nn=xmlNewChild(n, ns, BAD_CAST "DigestMethod", NULL);
  xmlNewProp(nn,
             BAD_CAST "Algorithm",
             BAD_CAST "http://www.w3.org/2000/09/xmldsig#sha1");

  return 0;
}



