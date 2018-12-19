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
#include <gwenhywfar/buffer.h>



typedef struct AB_IMEXPORTER_XML AB_IMEXPORTER_XML;
struct AB_IMEXPORTER_XML {
  int dummy;
};



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

static int AB_ImExporterXML_ImportDb(AB_IMEXPORTER *ie,
                                     AB_IMEXPORTER_CONTEXT *ctx,
                                     GWEN_DB_NODE *dbData,
                                     GWEN_DB_NODE *dbParams);



#endif /* AQBANKING_IMEX_XML_P_H */
