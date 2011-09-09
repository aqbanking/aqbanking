/***************************************************************************
 begin       : Mon Jan 07 2008
 copyright   : (C) 2008 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#ifndef AIO_OFX_OFXXMLCTX_L_H
#define AIO_OFX_OFXXMLCTX_L_H


#include "ofxgroup_l.h"

#include <aqbanking/imexporter.h>

#include <gwenhywfar/xmlctx.h>



GWEN_XML_CONTEXT *AIO_OfxXmlCtx_new(uint32_t flags, AB_IMEXPORTER_CONTEXT *ioContext);

const char *AIO_OfxXmlCtx_GetCharset(const GWEN_XML_CONTEXT *ctx);
void AIO_OfxXmlCtx_SetCharset(GWEN_XML_CONTEXT *ctx, const char *s);


int AIO_OfxXmlCtx_GetResultCode(const GWEN_XML_CONTEXT *ctx);
void AIO_OfxXmlCtx_SetResultCode(GWEN_XML_CONTEXT *ctx, int i);

const char *AIO_OfxXmlCtx_GetResultSeverity(const GWEN_XML_CONTEXT *ctx);
void AIO_OfxXmlCtx_SetResultSeverity(GWEN_XML_CONTEXT *ctx, const char *s);

AB_IMEXPORTER_CONTEXT *AIO_OfxXmlCtx_GetIoContext(const GWEN_XML_CONTEXT *ctx);

AIO_OFX_GROUP *AIO_OfxXmlCtx_GetCurrentGroup(const GWEN_XML_CONTEXT *ctx);

void AIO_OfxXmlCtx_SetCurrentGroup(GWEN_XML_CONTEXT *ctx, AIO_OFX_GROUP *g);

const char *AIO_OfxXmlCtx_GetCurrentTagName(const GWEN_XML_CONTEXT *ctx);
void AIO_OfxXmlCtx_SetCurrentTagName(GWEN_XML_CONTEXT *ctx, const char *s);


int AIO_OfxXmlCtx_SanitizeData(GWEN_XML_CONTEXT *ctx,
			       const char *data,
			       GWEN_BUFFER *buf);

#endif

