/***************************************************************************
    begin       : Mon Mar 01 2004
    copyright   : (C) 2018 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#ifndef AQBANKING_IMEX_XML_P_H
#define AQBANKING_IMEX_XML_P_H


#include "xml.h"

#include <aqbanking/imexporter_be.h>

#include <gwenhywfar/db.h>
#include <gwenhywfar/xml.h>



typedef struct AB_IMEXPORTER_XML AB_IMEXPORTER_XML;
struct AB_IMEXPORTER_XML {
  int dummy;
};



typedef struct {
  GWEN_XMLNODE *docRoot;            /* provided by caller (dont free) */
  GWEN_XMLNODE *currentDocNode;     /* pointer, dont free */

  GWEN_XMLNODE_LIST2 *xmlNodeStack;  /* do free */

  GWEN_DB_NODE *dbRoot;             /* provided by caller (dont free) */
  GWEN_DB_NODE *currentDbGroup;     /* pointer, dont free */

  GWEN_DB_NODE *tempDbRoot;         /* do free */
  GWEN_DB_NODE *currentTempDbGroup; /* pointer, dont free */
} AB_IMEXPORTER_XML_CONTEXT;




static void GWENHYWFAR_CB AB_ImExporterXML_FreeData(void *bp, void *p);


static int AB_ImExporterXML_Import(AB_IMEXPORTER *ie,
                                   AB_IMEXPORTER_CONTEXT *ctx,
                                   GWEN_SYNCIO *sio,
                                   GWEN_DB_NODE *params);

static int AB_ImExporterXML_Export(AB_IMEXPORTER *ie,
                                   AB_IMEXPORTER_CONTEXT *ctx,
                                   GWEN_SYNCIO *sio,
                                   GWEN_DB_NODE *params);

static int AB_ImExporterXML_CheckFile(AB_IMEXPORTER *ie, const char *fname);


static GWEN_XMLNODE *AB_ImExporterXML_ReadSchemaFile(AB_IMEXPORTER *ie, const char *schemaName);
static GWEN_XMLNODE *AB_ImExporterXML_ReadXmlFromSio(AB_IMEXPORTER *ie, GWEN_SYNCIO *sio);



static AB_IMEXPORTER_XML_CONTEXT *AB_ImExporterXML_Context_new(GWEN_XMLNODE *documentRoot, GWEN_DB_NODE *dbRoot);
static void AB_ImExporterXML_Context_free(AB_IMEXPORTER_XML_CONTEXT *ctx);

static void AB_ImExporterXML_Context_EnterDocNode(AB_IMEXPORTER_XML_CONTEXT *ctx, GWEN_XMLNODE *xmlNode);
static void AB_ImExporterXML_Context_LeaveDocNode(AB_IMEXPORTER_XML_CONTEXT *ctx);


static int AB_ImExporterXML_Import_ReplaceVars(const char *s, GWEN_DB_NODE *db, GWEN_BUFFER *dbuf);

static const char *AB_ImExporterXML_Import_GetCharValueByPath(GWEN_XMLNODE *xmlNode, const char *path, const char *defValue);


static int AB_ImExporterXML_Import_Handle_Enter(AB_IMEXPORTER_XML_CONTEXT *ctx, GWEN_XMLNODE *xmlNode);
static int AB_ImExporterXML_Import_Handle_ForEvery(AB_IMEXPORTER_XML_CONTEXT *ctx, GWEN_XMLNODE *xmlNode);
static int AB_ImExporterXML_Import_Handle_CreateAndEnterDbGroup(AB_IMEXPORTER_XML_CONTEXT *ctx, GWEN_XMLNODE *xmlNode);
static int AB_ImExporterXML_Import_Handle_CreateAndEnterTempDbGroup(AB_IMEXPORTER_XML_CONTEXT *ctx, GWEN_XMLNODE *xmlNode);
static int AB_ImExporterXML_Import_Handle_SetCharValue(AB_IMEXPORTER_XML_CONTEXT *ctx, GWEN_XMLNODE *xmlNode);
static int AB_ImExporterXML_Import_Handle_SetTempCharValue(AB_IMEXPORTER_XML_CONTEXT *ctx, GWEN_XMLNODE *xmlNode);
static int AB_ImExporterXML_Import_Handle_IfCharDataMatches(AB_IMEXPORTER_XML_CONTEXT *ctx, GWEN_XMLNODE *xmlNode);
static int AB_ImExporterXML_Import_Handle_IfNotCharDataMatches(AB_IMEXPORTER_XML_CONTEXT *ctx, GWEN_XMLNODE *xmlNode);
static int AB_ImExporterXML_Import_Handle_IfHasCharData(AB_IMEXPORTER_XML_CONTEXT *ctx, GWEN_XMLNODE *xmlNode);
static int AB_ImExporterXML_Import_Handle_IfNotHasCharData(AB_IMEXPORTER_XML_CONTEXT *ctx, GWEN_XMLNODE *xmlNode);
static int AB_ImExporterXML_Import_Handle_IfPathExists(AB_IMEXPORTER_XML_CONTEXT *ctx, GWEN_XMLNODE *xmlNode);
static int AB_ImExporterXML_Import_Handle_IfNotPathExists(AB_IMEXPORTER_XML_CONTEXT *ctx, GWEN_XMLNODE *xmlNode);

static int AB_ImExporterXML_Import_HandleChildren(AB_IMEXPORTER_XML_CONTEXT *ctx, GWEN_XMLNODE *xmlNode);



int AB_ImExporterXML_ImportToDb(GWEN_XMLNODE *xmlNodeDocument,
                                GWEN_XMLNODE *xmlNodeSchema,
                                GWEN_DB_NODE *dbDestination);















#endif /* AQBANKING_IMEX_XML_P_H */
