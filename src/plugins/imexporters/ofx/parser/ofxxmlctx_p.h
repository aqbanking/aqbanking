/***************************************************************************
 $RCSfile$
 -------------------
 begin       : Mon Jan 07 2008
 copyright   : (C) 2008 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#ifndef AIO_OFX_OFXXMLCTX_P_H
#define AIO_OFX_OFXXMLCTX_P_H


#include "ofxxmlctx_l.h"



typedef struct AIO_OFX_XMLCTX AIO_OFX_XMLCTX;
struct AIO_OFX_XMLCTX {
  /* answers for SIGNON */
  int resultCode;
  char *resultSeverity;

  AB_IMEXPORTER_CONTEXT *ioContext;

  AIO_OFX_GROUP *currentGroup;
  char *currentTagName;

  char *charset;

  int startDepthOfOfxElement;
};


static void GWENHYWFAR_CB AIO_OfxXmlCtx_FreeData(void *bp, void *p);


static int AIO_OfxXmlCtx_StartTag(GWEN_XML_CONTEXT *ctx, const char *tagName);
static int AIO_OfxXmlCtx_EndTag(GWEN_XML_CONTEXT *ctx, int closing);
static int AIO_OfxXmlCtx_AddData(GWEN_XML_CONTEXT *ctx, const char *data);
static int AIO_OfxXmlCtx_AddComment(GWEN_XML_CONTEXT *ctx, const char *data);
static int AIO_OfxXmlCtx_AddAttr(GWEN_XML_CONTEXT *ctx,
				 const char *attrName,
				 const char *attrData);

static int AIO_OfxXmlCtx_CleanupData(GWEN_XML_CONTEXT *ctx,
                                     const char *data,
                                     GWEN_BUFFER *buf);

#endif

