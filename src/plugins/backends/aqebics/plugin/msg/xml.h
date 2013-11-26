/***************************************************************************
    begin       : Mon Mar 01 2004
    copyright   : (C) 2004-2010 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifndef AQEBICS_MSG_XML_H
#define AQEBICS_MSG_XML_H


#include <aqebics/aqebics.h>

#include <gwenhywfar/path.h>
#include <gwenhywfar/buffer.h>
#include <gwenhywfar/mdigest.h>

#include <libxml/tree.h>
#include <libxml/parser.h>
#include <libxml/xpath.h>



xmlNodePtr EB_Xml_GetNode(xmlNodePtr n, const char *path,
                          uint32_t flags);

int EB_Xml_SetCharValue(xmlNodePtr n, const char *path, const char *value);
const char *EB_Xml_GetCharValue(xmlNodePtr n, const char *path,
                                const char *defValue);

int EB_Xml_SetIntValue(xmlNodePtr n, const char *path, int value);
int EB_Xml_GetIntValue(xmlNodePtr n, const char *path, int defValue);

int EB_Xml_Ebicsify(xmlNodePtr node, const char *hVersion);


int EB_Xml_CompressDoc(xmlDocPtr doc, GWEN_BUFFER *buf);
int EB_Xml_Compress64Doc(xmlDocPtr doc, GWEN_BUFFER *buf);

int EB_Xml_UncompressDoc(const char *ptr, int size, xmlDocPtr *pdoc);
int EB_Xml_Uncompress64Doc(const char *ptr, int size,
                           xmlDocPtr *pdoc);

int EB_Xml_InsertChild(xmlNodePtr node, xmlNodePtr n);


int EB_Xml_GetXpathData(xmlNodePtr signedInfoNode,
			const xmlChar *uri,
			GWEN_BUFFER *rbuf);

int EB_Xml_DocFromBuffer(const char *ptr, int size, xmlDocPtr *pdoc);


int EB_Xml_BuildHashData(xmlNodePtr signedInfoNode,
			 const xmlChar *uri,
			 GWEN_BUFFER *rbuf);

int EB_Xml_BuildNodeHash(xmlNodePtr node,
			 const char *uri,
			 GWEN_MDIGEST *md,
			 GWEN_BUFFER *hbuf);

int EB_Xml_BuildNodeHashSha1(xmlNodePtr node,
			     const char *uri,
			     GWEN_BUFFER *hbuf);

int EB_Xml_BuildNodeHashSha256(xmlNodePtr node,
			       const char *uri,
			       GWEN_BUFFER *hbuf);

int EB_Xml_BuildNodeHashSha256Sha256(xmlNodePtr node,
				     const char *uri,
				     GWEN_BUFFER *hbuf);



#endif
