/***************************************************************************
 begin       : Mon Jan 07 2008
 copyright   : (C) 2008,2012 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/



#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "ofxxmlctx_p.h"
#include "g_document_l.h"

#include <gwenhywfar/misc.h>
#include <gwenhywfar/debug.h>
#include <gwenhywfar/text.h>



GWEN_INHERIT(GWEN_XML_CONTEXT, AIO_OFX_XMLCTX)




GWEN_XML_CONTEXT *AIO_OfxXmlCtx_new(uint32_t flags, AB_IMEXPORTER_CONTEXT *ioContext) {
  GWEN_XML_CONTEXT *ctx;
  AIO_OFX_XMLCTX *xctx;
  AIO_OFX_GROUP *g;

  /* create base object */
  ctx=GWEN_XmlCtx_new(flags);
  assert(ctx);

  /* create and assign extension */
  GWEN_NEW_OBJECT(AIO_OFX_XMLCTX, xctx);
  assert(xctx);
  GWEN_INHERIT_SETDATA(GWEN_XML_CONTEXT, AIO_OFX_XMLCTX, ctx, xctx,
		       AIO_OfxXmlCtx_FreeData);
  xctx->ioContext=ioContext;

  /* set virtual functions */
  GWEN_XmlCtx_SetStartTagFn(ctx, AIO_OfxXmlCtx_StartTag);
  GWEN_XmlCtx_SetEndTagFn(ctx, AIO_OfxXmlCtx_EndTag);
  GWEN_XmlCtx_SetAddDataFn(ctx, AIO_OfxXmlCtx_AddData);
  GWEN_XmlCtx_SetAddCommentFn(ctx, AIO_OfxXmlCtx_AddComment);
  GWEN_XmlCtx_SetAddAttrFn(ctx, AIO_OfxXmlCtx_AddAttr);

  /* create initial group */
  g=AIO_OfxGroup_Document_new("OFX_FILE", NULL, ctx);
  assert(g);
  AIO_OfxXmlCtx_SetCurrentGroup(ctx, g);


  /* return base object */
  return ctx;
}



GWENHYWFAR_CB
void AIO_OfxXmlCtx_FreeData(void *bp, void *p) {
  AIO_OFX_XMLCTX *xctx;
  AIO_OFX_GROUP *g;

  xctx=(AIO_OFX_XMLCTX*)p;

  g=xctx->currentGroup;
  while (g) {
    AIO_OFX_GROUP *gParent;

    gParent=AIO_OfxGroup_GetParent(g);
    AIO_OfxGroup_free(g);
    g=gParent;
  }

  free(xctx->resultSeverity);
  free(xctx->currentTagName);

  free(xctx->charset);

  GWEN_FREE_OBJECT(xctx);
}



const char *AIO_OfxXmlCtx_GetCharset(const GWEN_XML_CONTEXT *ctx) {
  AIO_OFX_XMLCTX *xctx;

  assert(ctx);
  xctx=GWEN_INHERIT_GETDATA(GWEN_XML_CONTEXT, AIO_OFX_XMLCTX, ctx);
  assert(xctx);

  return xctx->charset;
}



void AIO_OfxXmlCtx_SetCharset(GWEN_XML_CONTEXT *ctx, const char *s) {
  AIO_OFX_XMLCTX *xctx;

  assert(ctx);
  xctx=GWEN_INHERIT_GETDATA(GWEN_XML_CONTEXT, AIO_OFX_XMLCTX, ctx);
  assert(xctx);

  free(xctx->charset);
  if (s) xctx->charset=strdup(s);
  else xctx->charset=NULL;
}



int AIO_OfxXmlCtx_GetResultCode(const GWEN_XML_CONTEXT *ctx) {
  AIO_OFX_XMLCTX *xctx;

  assert(ctx);
  xctx=GWEN_INHERIT_GETDATA(GWEN_XML_CONTEXT, AIO_OFX_XMLCTX, ctx);
  assert(xctx);

  return xctx->resultCode;
}



void AIO_OfxXmlCtx_SetResultCode(GWEN_XML_CONTEXT *ctx, int i) {
  AIO_OFX_XMLCTX *xctx;

  assert(ctx);
  xctx=GWEN_INHERIT_GETDATA(GWEN_XML_CONTEXT, AIO_OFX_XMLCTX, ctx);
  assert(xctx);

  xctx->resultCode=i;
}



const char *AIO_OfxXmlCtx_GetResultSeverity(const GWEN_XML_CONTEXT *ctx) {
  AIO_OFX_XMLCTX *xctx;

  assert(ctx);
  xctx=GWEN_INHERIT_GETDATA(GWEN_XML_CONTEXT, AIO_OFX_XMLCTX, ctx);
  assert(xctx);

  return xctx->resultSeverity;
}



void AIO_OfxXmlCtx_SetResultSeverity(GWEN_XML_CONTEXT *ctx, const char *s) {
  AIO_OFX_XMLCTX *xctx;

  assert(ctx);
  xctx=GWEN_INHERIT_GETDATA(GWEN_XML_CONTEXT, AIO_OFX_XMLCTX, ctx);
  assert(xctx);

  free(xctx->resultSeverity);
  if (s) xctx->resultSeverity=strdup(s);
  else xctx->resultSeverity=NULL;
}



AB_IMEXPORTER_CONTEXT*
AIO_OfxXmlCtx_GetIoContext(const GWEN_XML_CONTEXT *ctx){
  AIO_OFX_XMLCTX *xctx;

  assert(ctx);
  xctx=GWEN_INHERIT_GETDATA(GWEN_XML_CONTEXT, AIO_OFX_XMLCTX, ctx);
  assert(xctx);

  return xctx->ioContext;
}



AIO_OFX_GROUP *AIO_OfxXmlCtx_GetCurrentGroup(const GWEN_XML_CONTEXT *ctx) {
  AIO_OFX_XMLCTX *xctx;

  assert(ctx);
  xctx=GWEN_INHERIT_GETDATA(GWEN_XML_CONTEXT, AIO_OFX_XMLCTX, ctx);
  assert(xctx);

  return xctx->currentGroup;
}



void AIO_OfxXmlCtx_SetCurrentGroup(GWEN_XML_CONTEXT *ctx, AIO_OFX_GROUP *g){
  AIO_OFX_XMLCTX *xctx;

  assert(ctx);
  xctx=GWEN_INHERIT_GETDATA(GWEN_XML_CONTEXT, AIO_OFX_XMLCTX, ctx);
  assert(xctx);

  xctx->currentGroup=g;
}



const char *AIO_OfxXmlCtx_GetCurrentTagName(const GWEN_XML_CONTEXT *ctx) {
  AIO_OFX_XMLCTX *xctx;

  assert(ctx);
  xctx=GWEN_INHERIT_GETDATA(GWEN_XML_CONTEXT, AIO_OFX_XMLCTX, ctx);
  assert(xctx);

  return xctx->currentTagName;
}



void AIO_OfxXmlCtx_SetCurrentTagName(GWEN_XML_CONTEXT *ctx, const char *s) {
  AIO_OFX_XMLCTX *xctx;

  assert(ctx);
  xctx=GWEN_INHERIT_GETDATA(GWEN_XML_CONTEXT, AIO_OFX_XMLCTX, ctx);
  assert(xctx);

  free(xctx->currentTagName);
  if (s) xctx->currentTagName=strdup(s);
  else xctx->currentTagName=NULL;
}



int AIO_OfxXmlCtx_CleanupData(GWEN_XML_CONTEXT *ctx,
			      const char *data,
			      GWEN_BUFFER *buf) {
  const uint8_t *p;
  uint8_t *dst;
  uint8_t *src;
  unsigned int size;
  unsigned int i;
  int lastWasBlank;
  uint8_t *lastBlankPos;
  uint32_t bStart=0;

  if (GWEN_Text_UnescapeXmlToBuffer(data, buf)) {
    DBG_INFO(AQBANKING_LOGDOMAIN, "here");
    return GWEN_ERROR_BAD_DATA;
  }

  dst=(uint8_t*)GWEN_Buffer_GetStart(buf);
  src=dst;

  /* skip leading blanks */
  while(*src && (*src<33 || *src>=127))
    src++;

  p=src;
  bStart=src-((uint8_t*)GWEN_Buffer_GetStart(buf));
  size=GWEN_Buffer_GetUsedBytes(buf)-bStart;
  lastWasBlank=0;
  lastBlankPos=0;

  for (i=0; i<size; i++) {
    uint8_t c;

    c=*p;
    /* DISABLED: c>=127 would filter out umlauts...
     if (c<32 || c>=127)*/
    if (c<32)
      c=32;

    /* remember next loop whether this char was a blank */
    if (c==32) {
      if (!lastWasBlank) {
	/* store only one blank */
	lastWasBlank=1;
	lastBlankPos=dst;
	*(dst++)=c;
      }
    }
    else {
      lastWasBlank=0;
      lastBlankPos=0;
      *(dst++)=c;
    }
    p++;
  }

  /* remove trailing blanks */
  if (lastBlankPos!=0)
    dst=lastBlankPos;

  size=dst-(uint8_t*)GWEN_Buffer_GetStart(buf);
  GWEN_Buffer_Crop(buf, 0, size);

  return 0;
}



int AIO_OfxXmlCtx_SanitizeData(GWEN_XML_CONTEXT *ctx,
			       const char *data,
			       GWEN_BUFFER *buf) {
  AIO_OFX_XMLCTX *xctx;

  assert(ctx);
  xctx=GWEN_INHERIT_GETDATA(GWEN_XML_CONTEXT, AIO_OFX_XMLCTX, ctx);
  assert(xctx);

  if (xctx->charset) {
    GWEN_BUFFER *tbuf;
    int rv;

    tbuf=GWEN_Buffer_new(0, 256, 0, 1);
    rv=AIO_OfxXmlCtx_CleanupData(ctx, data, tbuf);
    if (rv<0) {
      DBG_INFO(AQBANKING_LOGDOMAIN, "here (%d)", rv);
      GWEN_Buffer_free(tbuf);
      return rv;
    }

    rv=GWEN_Text_ConvertCharset(xctx->charset, "UTF-8",
				GWEN_Buffer_GetStart(tbuf),
				GWEN_Buffer_GetUsedBytes(tbuf),
				buf);
    if (rv<0) {
      DBG_INFO(AQBANKING_LOGDOMAIN, "here (%d)", rv);
      GWEN_Buffer_free(tbuf);
      return rv;
    }
    GWEN_Buffer_free(tbuf);
    return 0;
  }
  else
    return AIO_OfxXmlCtx_CleanupData(ctx, data, buf);
}









int AIO_OfxXmlCtx_StartTag(GWEN_XML_CONTEXT *ctx, const char *tagName) {
  AIO_OFX_XMLCTX *xctx;

  assert(ctx);
  xctx=GWEN_INHERIT_GETDATA(GWEN_XML_CONTEXT, AIO_OFX_XMLCTX, ctx);
  assert(xctx);

  DBG_INFO(AQBANKING_LOGDOMAIN, "Starting tag [%s]", tagName);

  /* store for later, do nothing more here */
  AIO_OfxXmlCtx_SetCurrentTagName(ctx, tagName);

  return 0;
}



int AIO_OfxXmlCtx_EndTag(GWEN_XML_CONTEXT *ctx, int closing) {
  AIO_OFX_XMLCTX *xctx;

  assert(ctx);
  xctx=GWEN_INHERIT_GETDATA(GWEN_XML_CONTEXT, AIO_OFX_XMLCTX, ctx);
  assert(xctx);

  if (closing) {
    /* just ignore empty tags which are closed immediately */
    DBG_INFO(AQBANKING_LOGDOMAIN, "Closing empty tag [%s]",
	     (xctx->currentTagName)?xctx->currentTagName:"<noname>");
    return 0;
  }

  if (xctx->currentTagName==NULL) {
    DBG_INFO(AQBANKING_LOGDOMAIN, "No tag name, malformed OFX file");
    return GWEN_ERROR_BAD_DATA;
  }

  DBG_INFO(AQBANKING_LOGDOMAIN, "Completed tag [%s]", xctx->currentTagName);

  if (xctx->currentGroup) {
    if (*(xctx->currentTagName)=='/') {
      int rv;
      int endingOfxDoc=0;

      if (strcasecmp(xctx->currentTagName, "/OFX")==0) {
	DBG_INFO(AQBANKING_LOGDOMAIN, "End of OFX document reached, will reset depth to %d",
		 xctx->startDepthOfOfxElement);
	endingOfxDoc=1;
      }

      /* it is a closing tag, call EndTagFn */
      DBG_INFO(AQBANKING_LOGDOMAIN,
	       "Calling %s->EndTag(%s)",
	       AIO_OfxGroup_GetGroupName(xctx->currentGroup),
	       xctx->currentTagName);
      rv=AIO_OfxGroup_EndTag(xctx->currentGroup, xctx->currentTagName+1);
      if (rv<0) {
	if (rv!=GWEN_ERROR_NOT_IMPLEMENTED) {
	  DBG_INFO(AQBANKING_LOGDOMAIN,
		   "Error in EndTag(%s) for [%s]",
		   AIO_OfxGroup_GetGroupName(xctx->currentGroup),
		   xctx->currentTagName);
	  return rv;
	}
      }
      else if (rv==1) {
        AIO_OFX_GROUP *g;
	AIO_OFX_GROUP *gParent;

	/* pop current group from stack */
	g=xctx->currentGroup;
	gParent=AIO_OfxGroup_GetParent(g);
	xctx->currentGroup=gParent;
	if (gParent) {
	  DBG_INFO(AQBANKING_LOGDOMAIN,
		   "Calling %s->EndSubGroup(%s)",
		   AIO_OfxGroup_GetGroupName(gParent),
                   AIO_OfxGroup_GetGroupName(g));
	  AIO_OfxGroup_EndSubGroup(gParent, g);
	}
	AIO_OfxGroup_free(g);
	GWEN_XmlCtx_DecDepth(ctx);
      }

      if (endingOfxDoc) {
	/* TODO: Tags which have no closing element should decrease the depth by themselves... */
	DBG_INFO(AQBANKING_LOGDOMAIN, "End of OFX document reached, resetting depth to %d",
		 xctx->startDepthOfOfxElement);
	GWEN_XmlCtx_SetDepth(ctx, xctx->startDepthOfOfxElement);
      }
    }
    else {
      int rv;

      if (strcasecmp(xctx->currentTagName, "OFX")==0) {
	DBG_INFO(AQBANKING_LOGDOMAIN, "Start of OFX document reached, storing depth");
	xctx->startDepthOfOfxElement=GWEN_XmlCtx_GetDepth(ctx);
      }

      /* it is an opening tag, call StartTagFn */
      DBG_INFO(AQBANKING_LOGDOMAIN,
	       "Calling %s->StartTag(%s)",
	       AIO_OfxGroup_GetGroupName(xctx->currentGroup),
	       xctx->currentTagName);
      rv=AIO_OfxGroup_StartTag(xctx->currentGroup, xctx->currentTagName);
      if (rv<0) {
	if (rv!=GWEN_ERROR_NOT_IMPLEMENTED) {
	  DBG_INFO(AQBANKING_LOGDOMAIN,
		   "Error in StartTag(%s) for [%s]",
		   AIO_OfxGroup_GetGroupName(xctx->currentGroup),
		   xctx->currentTagName);
	  return rv;
	}
      }
    }
  }

  return 0;
}






int AIO_OfxXmlCtx_AddData(GWEN_XML_CONTEXT *ctx, const char *data) {
  AIO_OFX_XMLCTX *xctx;

  assert(ctx);
  xctx=GWEN_INHERIT_GETDATA(GWEN_XML_CONTEXT, AIO_OFX_XMLCTX, ctx);
  assert(xctx);

  if (xctx->currentGroup) {
    int rv;

    DBG_INFO(AQBANKING_LOGDOMAIN,
	     "Calling %s->AddData()",
	     AIO_OfxGroup_GetGroupName(xctx->currentGroup));
    rv=AIO_OfxGroup_AddData(xctx->currentGroup, data);
    if (rv<0) {
      if (rv!=GWEN_ERROR_NOT_IMPLEMENTED) {
	DBG_INFO(AQBANKING_LOGDOMAIN,
		 "Error in AddData(%s)",
		 AIO_OfxGroup_GetGroupName(xctx->currentGroup));
	return rv;
      }
    }
  }

  return 0;
}



int AIO_OfxXmlCtx_AddComment(GWEN_XML_CONTEXT *ctx, const char *data) {
  AIO_OFX_XMLCTX *xctx;

  assert(ctx);
  xctx=GWEN_INHERIT_GETDATA(GWEN_XML_CONTEXT, AIO_OFX_XMLCTX, ctx);
  assert(xctx);

  /* ignore comments */
  return 0;
}



int AIO_OfxXmlCtx_AddAttr(GWEN_XML_CONTEXT *ctx,
			  const char *attrName,
			  const char *attrData) {
  AIO_OFX_XMLCTX *xctx;

  assert(ctx);
  xctx=GWEN_INHERIT_GETDATA(GWEN_XML_CONTEXT, AIO_OFX_XMLCTX, ctx);
  assert(xctx);

  /* ignore attributes */
  return 0;
}





