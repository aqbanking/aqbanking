/***************************************************************************
 $RCSfile: adminjobs.h,v $
                             -------------------
    cvs         : $Id: adminjobs.h,v 1.3 2006/01/13 13:59:58 cstim Exp $
    begin       : Mon Mar 01 2004
    copyright   : (C) 2004 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "xml_p.h"
#include "zip.h"

#include <gwenhywfar/debug.h>
#include <gwenhywfar/base64.h>
#include <gwenhywfar/text.h>

#include <xmlsec/transforms.h>
#include <xmlsec/errors.h>




void* EB_Xml__HandlePath(const char *entry,
                         void *data,
                         int idx,
                         uint32_t flags) {
  xmlNodePtr n;
  xmlNodePtr nn;
  int i;
  xmlNsPtr nameSpace=NULL;
  const char *p;
  const char *name;

  n=(xmlNodePtr)data;

  name=entry;
  p=strchr(entry, ':');
  if (p) {
    char prefix[32];
    int plen;

    plen=p-entry;
    if (plen) {
      if (plen>=sizeof(prefix)) {
        DBG_ERROR(AQEBICS_LOGDOMAIN,
                  "Prefix too long (%d>%d)", (int)plen, (int)sizeof(prefix));
        return 0;
      }
      strncpy(prefix, entry, plen);
      prefix[plen]=0;
      nameSpace=xmlSearchNs(n->doc, n, BAD_CAST prefix);
      if (!nameSpace) {
        DBG_ERROR(AQEBICS_LOGDOMAIN, "Namespace \"%s\" not found", prefix);
        return 0;
      }
    }
    name=p+1;
  }

  /* check whether we are allowed to simply create the node */
  if (
      ((flags & GWEN_PATH_FLAGS_LAST) &&
       (((flags & GWEN_PATH_FLAGS_VARIABLE) &&
         (flags & GWEN_PATH_FLAGS_CREATE_VAR)) ||
        (!(flags & GWEN_PATH_FLAGS_VARIABLE) &&
         (flags & GWEN_PATH_FLAGS_CREATE_GROUP)))
      ) ||
      (
       !(flags & GWEN_PATH_FLAGS_LAST) &&
       (flags & GWEN_PATH_FLAGS_PATHCREATE))
     ) {
    /* simply create the new variable/group */
    if (idx!=0) {
      DBG_ERROR(AQEBICS_LOGDOMAIN, "Index is not 0, not creating %s[%d]",
               entry, idx);
      return 0;
    }
    DBG_VERBOUS(AQEBICS_LOGDOMAIN,
                "Unconditionally creating entry \"%s\"", entry);
    nn=xmlNewChild(n, nameSpace, BAD_CAST name, NULL);
    return nn;
  }

  /* find the node */

  nn=n->children;
  i=idx;
  while(nn) {
    if (nn->type==XML_ELEMENT_NODE) {
      if (nn->name && strcmp((const char*)nn->name, name)==0) {
        if (i--==0)
          break;
      }
    }
    nn=nn->next;
  } /* while */

  if (!nn) {
    /* node not found, check, if we are allowed to create it */
    if (
        (!(flags & GWEN_PATH_FLAGS_LAST) &&
         (flags & GWEN_PATH_FLAGS_PATHMUSTEXIST)) ||
        (flags & GWEN_PATH_FLAGS_NAMEMUSTEXIST)
       ) {
      DBG_VERBOUS(AQEBICS_LOGDOMAIN,
                  "Entry \"%s\" does not exist", entry);
      return 0;
    }
    /* create the new variable/group */
    if (idx!=0) {
      DBG_INFO(AQEBICS_LOGDOMAIN, "Index is not 0, not creating %s[%d]",
               entry, idx);
      return 0;
    }
    DBG_VERBOUS(AQEBICS_LOGDOMAIN,
                "Entry \"%s\" not found, creating", entry);
    nn=xmlNewChild(n, nameSpace, BAD_CAST name, NULL);
  } /* if node not found */
  else {
    /* node does exist, check whether this is ok */
    if (
        ((flags & GWEN_PATH_FLAGS_LAST) &&
         (flags & GWEN_PATH_FLAGS_NAMEMUSTNOTEXIST)) ||
        (!(flags & GWEN_PATH_FLAGS_LAST) &&
         (flags & GWEN_PATH_FLAGS_PATHMUSTNOTEXIST))
       ) {
      DBG_VERBOUS(AQEBICS_LOGDOMAIN, "Entry \"%s\" already exists", entry);
      return 0;
    }
  }

  return nn;
}



xmlNodePtr EB_Xml_GetNode(xmlNodePtr n, const char *path,
                          uint32_t flags) {
  return (xmlNodePtr)GWEN_Path_HandleWithIdx(path,
                                             n,
                                             flags,
                                             EB_Xml__HandlePath);
}



int EB_Xml_SetCharValue(xmlNodePtr n, const char *path, const char *value) {
  xmlNodePtr node;

  node=EB_Xml_GetNode(n, path, 0);
  if (!node) {
    DBG_INFO(AQEBICS_LOGDOMAIN, "here");
    return -1;
  }

  xmlNodeSetContent(node, BAD_CAST value);

  return 0;
}



const char *EB_Xml_GetCharValue(xmlNodePtr n, const char *path,
                                const char *defValue) {
  xmlNodePtr node;

  node=EB_Xml_GetNode(n, path, GWEN_PATH_FLAGS_NAMEMUSTEXIST);
  if (!node) {
    DBG_INFO(AQEBICS_LOGDOMAIN, "path [%s] not found", path);
    return defValue;
  }
  node=node->children;
  if (node==0)
    return defValue;

  while(node) {
    if (node->type==XML_TEXT_NODE) {
      return (const char*)node->content;
    }
    node=node->next;
  }

  return defValue;
}



int EB_Xml_SetIntValue(xmlNodePtr n, const char *path, int value) {
  char numbuf[32];

  snprintf(numbuf, sizeof(numbuf)-1, "%d", value);
  numbuf[sizeof(numbuf)-1]=0;
  return EB_Xml_SetCharValue(n, path, numbuf);
}



int EB_Xml_GetIntValue(xmlNodePtr n, const char *path, int defValue) {
  const char *s;
  int i;

  s=EB_Xml_GetCharValue(n, path, NULL);
  if (s==NULL)
    return defValue;
  if (1!=sscanf(s, "%i", &i))
    return defValue;
  return i;
}



int EB_Xml_CompressDoc(xmlDocPtr doc, GWEN_BUFFER *buf) {
  xmlChar *xmlbuff;
  int buffersize;
  int rv;

  xmlDocDumpFormatMemory(doc, &xmlbuff, &buffersize, 1);
  if (buffersize==0) {
    DBG_ERROR(AQEBICS_LOGDOMAIN, "Empty doc");
    return -1;
  }

#if 0
  if (1) {
    FILE *f;

    f=fopen("/tmp/compress.txt", "w+");
    if (f) {
      fwrite((const char*)xmlbuff, buffersize, 1, f);
      fclose(f);
    }
    else {
      DBG_ERROR(0, "Could not create file");
      assert(0);
    }
  }
#endif

  rv=EB_Zip_Deflate((const char*)xmlbuff, buffersize, buf);
  xmlFree(xmlbuff);
  if (rv) {
    DBG_ERROR(AQEBICS_LOGDOMAIN, "Could not zip doc (%d)", rv);
    return -1;
  }

  return 0;
}



int EB_Xml_Compress64Doc(xmlDocPtr doc, GWEN_BUFFER *buf) {
  GWEN_BUFFER *tbuf;
  int rv;

  tbuf=GWEN_Buffer_new(0, 512, 0, 1);
  rv=EB_Xml_CompressDoc(doc, tbuf);
  if (rv) {
    GWEN_Buffer_free(tbuf);
    return rv;
  }

  rv=GWEN_Base64_Encode((const unsigned char*)GWEN_Buffer_GetStart(tbuf),
                        GWEN_Buffer_GetUsedBytes(tbuf),
                        buf, 0);
  GWEN_Buffer_free(tbuf);
  if (rv) {
    DBG_ERROR(AQEBICS_LOGDOMAIN, "Could not base64-encode order");
    return -1;
  }

  return 0;
}



int EB_Xml_UncompressDoc(const char *ptr, int size, xmlDocPtr *pdoc) {
  xmlDocPtr doc;
  int rv;
  GWEN_BUFFER *tbuf;

  tbuf=GWEN_Buffer_new(0, (size*3)/4, 0, 1);
  rv=EB_Zip_Inflate(ptr, size, tbuf);
  if (rv) {
    DBG_ERROR(AQEBICS_LOGDOMAIN, "Could not unzip doc (%d)", rv);
    GWEN_Buffer_free(tbuf);
    return -1;
  }

  doc=xmlParseMemory(GWEN_Buffer_GetStart(tbuf),
                     GWEN_Buffer_GetUsedBytes(tbuf));
  if (doc==NULL) {
    DBG_ERROR(AQEBICS_LOGDOMAIN, "Not an XML doc");
    GWEN_Buffer_Dump(tbuf, 2);
    GWEN_Buffer_free(tbuf);
    return -1;
  }
  GWEN_Buffer_free(tbuf);

  *pdoc=doc;

  return 0;
}



int EB_Xml_Uncompress64Doc(const char *ptr, int size,
                           xmlDocPtr *pdoc) {
  GWEN_BUFFER *tbuf;
  int rv;

  tbuf=GWEN_Buffer_new(0, 512, 0, 1);
  rv=GWEN_Base64_Decode((const unsigned char*)ptr, size, tbuf);
  if (rv) {
    DBG_ERROR(AQEBICS_LOGDOMAIN, "Could not base64-decode doc");
    GWEN_Buffer_free(tbuf);
    return -1;
  }

  rv=EB_Xml_UncompressDoc(GWEN_Buffer_GetStart(tbuf),
                          GWEN_Buffer_GetUsedBytes(tbuf),
                          pdoc);
  GWEN_Buffer_free(tbuf);
  if (rv) {
    DBG_INFO(AQEBICS_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }

  return 0;
}



int EB_Xml_Ebicsify(xmlNodePtr node, const char *hVersion) {
  if (strcasecmp(hVersion, "H002")==0) {
    xmlNsPtr ns;

    ns=xmlNewNs(node,
		BAD_CAST "http://www.ebics.org/H002",
		NULL);
    assert(ns);
    ns=xmlNewNs(node,
		BAD_CAST "http://www.w3.org/2000/09/xmldsig#",
		BAD_CAST "ds");
    assert(ns);
    ns=xmlNewNs(node,
		BAD_CAST "http://www.w3.org/2001/XMLSchema-instance",
		BAD_CAST "xsi");
    xmlNewNsProp(node,
		 ns,
		 BAD_CAST "schemaLocation", /* xsi:schemaLocation */
		 BAD_CAST "http://www.ebics.org/H002 "
		 "http://www.ebics.org/H002/ebics_request.xsd");
    xmlNewProp(node, BAD_CAST "Version", BAD_CAST "H002");
    xmlNewProp(node, BAD_CAST "Revision", BAD_CAST "1");
  }
  else if (strcasecmp(hVersion, "H003")==0) {
    xmlNsPtr ns;

    ns=xmlNewNs(node,
		BAD_CAST "http://www.ebics.org/H003",
		NULL);
    assert(ns);
    ns=xmlNewNs(node,
		BAD_CAST "http://www.w3.org/2000/09/xmldsig#",
		BAD_CAST "ds");
    assert(ns);
    ns=xmlNewNs(node,
		BAD_CAST "http://www.w3.org/2001/XMLSchema-instance",
		BAD_CAST "xsi");
    xmlNewNsProp(node,
		 ns,
		 BAD_CAST "schemaLocation", /* xsi:schemaLocation */
		 BAD_CAST "http://www.ebics.org/H003 "
		 "http://www.ebics.org/H003/ebics_request.xsd");
    xmlNewProp(node, BAD_CAST "Version", BAD_CAST "H003");
    xmlNewProp(node, BAD_CAST "Revision", BAD_CAST "1");
  }
  else {
    DBG_ERROR(AQEBICS_LOGDOMAIN, "Unknown EBICS version [%s]", hVersion);
    return -1;
  }

  return 0;
}



int EB_Xml_InsertChild(xmlNodePtr node, xmlNodePtr n) {
  if (node->children)
    xmlAddPrevSibling(node->children, n);
  else
    xmlAddChild(node, n);

  return 0;
}



int EB_Xml_GetXpathData(xmlNodePtr signedInfoNode,
			const xmlChar *uri,
			GWEN_BUFFER *rbuf) {
  xmlSecTransformPtr tptr;
  int rv;
  xmlSecTransformDataType firstType;
  xmlSecByte *result;
  xmlSecSize rlen;
  xmlSecTransformCtxPtr ctx;

  ctx=xmlSecTransformCtxCreate();
  assert(ctx);

  /* xpath (see xmlsec/transforms.h) */
  rv=xmlSecTransformCtxSetUri(ctx, uri, signedInfoNode);
  if (rv) {
    xmlSecError(XMLSEC_ERRORS_HERE,
		NULL,
		"xmlSecTransformCtxAppend",
		XMLSEC_ERRORS_R_XMLSEC_FAILED,
		XMLSEC_ERRORS_NO_MESSAGE);
    xmlSecTransformCtxDestroy(ctx);
    return (-1);
  }

  /* canonicalisation (see xmlsec/transforms.h) */
  tptr=xmlSecTransformCtxCreateAndAppend(ctx,
					 xmlSecTransformInclC14NId);
  if(tptr == NULL) {
    xmlSecError(XMLSEC_ERRORS_HERE,
		NULL,
		"xmlSecTransformCtxAppend",
		XMLSEC_ERRORS_R_XMLSEC_FAILED,
		XMLSEC_ERRORS_NO_MESSAGE);
    xmlSecTransformCtxDestroy(ctx);
    return(-1);
  }

  firstType=xmlSecTransformGetDataType(ctx->first,
				       xmlSecTransformModePush,
				       ctx);
  if((firstType & xmlSecTransformDataTypeXml) != 0) {
    xmlSecNodeSetPtr nodeset = NULL;

    nodeset=xmlSecNodeSetGetChildren(signedInfoNode->doc,
				     signedInfoNode, 1, 0);

    /* calculate the signature */
    rv=xmlSecTransformCtxXmlExecute(ctx, nodeset);
    if(rv<0) {
      xmlSecError(XMLSEC_ERRORS_HERE,
		  NULL,
		  "xmlSecTransformCtxXmlExecute",
		  XMLSEC_ERRORS_R_XMLSEC_FAILED,
		  XMLSEC_ERRORS_NO_MESSAGE);
      xmlSecNodeSetDestroy(nodeset);
      xmlSecTransformCtxDestroy(ctx);
      return(-1);
    }
    xmlSecNodeSetDestroy(nodeset);
  }
  else {
    xmlSecError(XMLSEC_ERRORS_HERE,
		NULL,
		"the binary c14n transforms are not supported yet",
		XMLSEC_ERRORS_R_NOT_IMPLEMENTED,
		XMLSEC_ERRORS_NO_MESSAGE);
    xmlSecTransformCtxDestroy(ctx);
    return(-1);
  }

  /* ctx->result now contains the resulting data */
  result=xmlSecBufferGetData(ctx->result);
  rlen=xmlSecBufferGetSize(ctx->result);
  if (result && rlen) {
    DBG_DEBUG(AQEBICS_LOGDOMAIN, "Have data: %d bytes", rlen);
    GWEN_Buffer_AppendBytes(rbuf, (char*)result, rlen);
  }
  xmlSecTransformCtxDestroy(ctx);

  return(0);
}



int EB_Xml_DocFromBuffer(const char *ptr, int size, xmlDocPtr *pdoc) {
  xmlDocPtr doc;

  doc=xmlParseMemory(ptr, size);
  if (doc==NULL) {
    DBG_ERROR(AQEBICS_LOGDOMAIN, "Not an XML doc");
    GWEN_Text_DumpString(ptr, (uint32_t) size, 2);
    return GWEN_ERROR_BAD_DATA;
  }

  *pdoc=doc;

  return 0;
}



int EB_Xml_BuildHashData(xmlNodePtr signedInfoNode,
			 const xmlChar *uri,
			 GWEN_BUFFER *rbuf) {
  xmlSecTransformPtr tptr;
  int rv;
  xmlSecTransformDataType firstType;
  xmlSecByte *result;
  xmlSecSize rlen;
  xmlSecTransformCtxPtr ctx;

  ctx=xmlSecTransformCtxCreate();
  assert(ctx);

  /* xpath (see xmlsec/transforms.h) */
  rv=xmlSecTransformCtxSetUri(ctx, uri, signedInfoNode);
  if (rv) {
    xmlSecError(XMLSEC_ERRORS_HERE,
		NULL,
		"xmlSecTransformCtxAppend",
		XMLSEC_ERRORS_R_XMLSEC_FAILED,
		XMLSEC_ERRORS_NO_MESSAGE);
    xmlSecTransformCtxDestroy(ctx);
    return (-1);
  }

  /* canonicalisation (see xmlsec/transforms.h) */
  tptr=xmlSecTransformCtxCreateAndAppend(ctx,
					 xmlSecTransformInclC14NId);
  if(tptr == NULL) {
    xmlSecError(XMLSEC_ERRORS_HERE,
		NULL,
		"xmlSecTransformCtxAppend",
		XMLSEC_ERRORS_R_XMLSEC_FAILED,
		XMLSEC_ERRORS_NO_MESSAGE);
    xmlSecTransformCtxDestroy(ctx);
    return(-1);
  }

#if 0
  /* hashing (see xmlsec/app.h)  */
  tptr=xmlSecTransformCtxCreateAndAppend(ctx,
					 xmlSecTransformHmacSha1Id);
  if(tptr == NULL) {
    xmlSecError(XMLSEC_ERRORS_HERE,
		NULL,
		"xmlSecTransformCtxAppend",
		XMLSEC_ERRORS_R_XMLSEC_FAILED,
		XMLSEC_ERRORS_NO_MESSAGE);
    xmlSecTransformCtxDestroy(ctx);
    return(-1);
  }
#endif

  firstType=xmlSecTransformGetDataType(ctx->first,
				       xmlSecTransformModePush,
				       ctx);
  if((firstType & xmlSecTransformDataTypeXml) != 0) {
    xmlSecNodeSetPtr nodeset = NULL;

    nodeset=xmlSecNodeSetGetChildren(signedInfoNode->doc,
				     signedInfoNode, 1, 0);

    /* calculate the signature */
    rv=xmlSecTransformCtxXmlExecute(ctx, nodeset);
    if(rv<0) {
      xmlSecError(XMLSEC_ERRORS_HERE,
		  NULL,
		  "xmlSecTransformCtxXmlExecute",
		  XMLSEC_ERRORS_R_XMLSEC_FAILED,
		  XMLSEC_ERRORS_NO_MESSAGE);
      xmlSecNodeSetDestroy(nodeset);
      xmlSecTransformCtxDestroy(ctx);
      return(-1);
    }
    xmlSecNodeSetDestroy(nodeset);
  }
  else {
    xmlSecError(XMLSEC_ERRORS_HERE,
		NULL,
		"the binary c14n transforms are not supported yet",
		XMLSEC_ERRORS_R_NOT_IMPLEMENTED,
		XMLSEC_ERRORS_NO_MESSAGE);
    xmlSecTransformCtxDestroy(ctx);
    return(-1);
  }

  /* ctx->result now contains the resulting data */
  result=xmlSecBufferGetData(ctx->result);
  rlen=xmlSecBufferGetSize(ctx->result);
  if (result && rlen) {
    GWEN_Buffer_AppendBytes(rbuf, (char*)result, rlen);
  }
  xmlSecTransformCtxDestroy(ctx);

  return(0);
}



int EB_Xml_BuildNodeHash(xmlNodePtr node,
			 const char *uri,
                         GWEN_MDIGEST *md,
			 GWEN_BUFFER *hbuf) {
  GWEN_BUFFER *dbuf;
  int rv;

  dbuf=GWEN_Buffer_new(0, 1024, 0, 1);
  rv=EB_Xml_BuildHashData(node, BAD_CAST uri, dbuf);
  if (rv) {
    DBG_INFO(AQEBICS_LOGDOMAIN, "here (%d)", rv);
    GWEN_Buffer_free(dbuf);
    return rv;
  }

  /* begin hash */
  rv=GWEN_MDigest_Begin(md);
  if (rv<0) {
    DBG_ERROR(AQEBICS_LOGDOMAIN, "here (%d)", rv);
    GWEN_Buffer_free(dbuf);
    return EB_RC_INTERNAL_ERROR;
  }

  /* update */
  rv=GWEN_MDigest_Update(md,
			 (const uint8_t*) GWEN_Buffer_GetStart(dbuf),
			 GWEN_Buffer_GetUsedBytes(dbuf));
  if (rv<0) {
    DBG_ERROR(AQEBICS_LOGDOMAIN, "here (%d)", rv);
    GWEN_Buffer_free(dbuf);
    return EB_RC_INTERNAL_ERROR;
  }

  /* end */
  rv=GWEN_MDigest_End(md);
  if (rv<0) {
    DBG_ERROR(AQEBICS_LOGDOMAIN, "here (%d)", rv);
    GWEN_Buffer_free(dbuf);
    return EB_RC_INTERNAL_ERROR;
  }

  GWEN_Buffer_AppendBytes(hbuf,
			  (const char*)GWEN_MDigest_GetDigestPtr(md),
			  GWEN_MDigest_GetDigestSize(md));
  /* cleanup */
  GWEN_Buffer_free(dbuf);

  return 0;
}



int EB_Xml_BuildNodeHashSha1(xmlNodePtr node,
			     const char *uri,
			     GWEN_BUFFER *hbuf) {
  GWEN_MDIGEST *md;
  int rv;

  md=GWEN_MDigest_Sha1_new();
  rv=EB_Xml_BuildNodeHash(node, uri, md, hbuf);
  GWEN_MDigest_free(md);
  return rv;
}



int EB_Xml_BuildNodeHashSha256(xmlNodePtr node,
			       const char *uri,
			       GWEN_BUFFER *hbuf) {
  GWEN_MDIGEST *md;
  int rv;

  md=GWEN_MDigest_Sha256_new();
  rv=EB_Xml_BuildNodeHash(node, uri, md, hbuf);
  GWEN_MDigest_free(md);
  return rv;
}



int EB_Xml_BuildNodeHashSha256Sha256(xmlNodePtr node,
				     const char *uri,
				     GWEN_BUFFER *hbuf) {
  GWEN_MDIGEST *md;
  int rv;
  GWEN_BUFFER *xbuf;

  xbuf=GWEN_Buffer_new(0, 64, 0, 1);
  md=GWEN_MDigest_Sha256_new();
  rv=EB_Xml_BuildNodeHash(node, uri, md, xbuf);
  if (rv<0) {
    DBG_INFO(AQEBICS_LOGDOMAIN, "here (%d)", rv);
    GWEN_Buffer_free(xbuf);
    GWEN_MDigest_free(md);
    return rv;
  }

  /* begin hash */
  rv=GWEN_MDigest_Begin(md);
  if (rv<0) {
    DBG_ERROR(AQEBICS_LOGDOMAIN, "here (%d)", rv);
    GWEN_Buffer_free(xbuf);
    return EB_RC_INTERNAL_ERROR;
  }

  /* update */
  rv=GWEN_MDigest_Update(md,
			 (const uint8_t*) GWEN_Buffer_GetStart(xbuf),
			 GWEN_Buffer_GetUsedBytes(xbuf));
  if (rv<0) {
    DBG_ERROR(AQEBICS_LOGDOMAIN, "here (%d)", rv);
    GWEN_Buffer_free(xbuf);
    return EB_RC_INTERNAL_ERROR;
  }

  /* end */
  rv=GWEN_MDigest_End(md);
  if (rv<0) {
    DBG_ERROR(AQEBICS_LOGDOMAIN, "here (%d)", rv);
    GWEN_Buffer_free(xbuf);
    return EB_RC_INTERNAL_ERROR;
  }
  GWEN_Buffer_free(xbuf);

  GWEN_Buffer_AppendBytes(hbuf,
			  (const char*)GWEN_MDigest_GetDigestPtr(md),
			  GWEN_MDigest_GetDigestSize(md));

  GWEN_MDigest_free(md);
  return rv;
}





