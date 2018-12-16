/***************************************************************************
    begin       : Sun Dec 16 2018
    copyright   : (C) 2018 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "xml_p.h"
#include "i18n_l.h"

#include <aqbanking/banking.h>
#include <aqbanking/banking_be.h>

#include <gwenhywfar/debug.h>
#include <gwenhywfar/misc.h>
#include <gwenhywfar/gui.h>
#include <gwenhywfar/inherit.h>




GWEN_INHERIT(AB_IMEXPORTER, AB_IMEXPORTER_XML);



AB_IMEXPORTER *AB_ImExporterXML_new(AB_BANKING *ab){
  AB_IMEXPORTER *ie;
  AB_IMEXPORTER_XML *ieh;

  ie=AB_ImExporter_new(ab, "xml");
  GWEN_NEW_OBJECT(AB_IMEXPORTER_XML, ieh);
  GWEN_INHERIT_SETDATA(AB_IMEXPORTER, AB_IMEXPORTER_XML, ie, ieh, AB_ImExporterXML_FreeData);

  AB_ImExporter_SetImportFn(ie, AB_ImExporterXML_Import);
  AB_ImExporter_SetExportFn(ie, AB_ImExporterXML_Export);
  AB_ImExporter_SetCheckFileFn(ie, AB_ImExporterXML_CheckFile);
  return ie;
}



void GWENHYWFAR_CB AB_ImExporterXML_FreeData(void *bp, void *p){
  AB_IMEXPORTER_XML *ieh;

  ieh=(AB_IMEXPORTER_XML*)p;

  GWEN_FREE_OBJECT(ieh);
}




int AB_ImExporterXML_Import(AB_IMEXPORTER *ie,
                            AB_IMEXPORTER_CONTEXT *ctx,
                            GWEN_SYNCIO *sio,
                            GWEN_DB_NODE *dbParams){
  GWEN_DB_NODE *dbSubParams;
  AB_IMEXPORTER_XML *ieh;
  const char *schemaName;
  GWEN_XMLNODE *xmlDocSchema;
  GWEN_XMLNODE *xmlDocData;
  GWEN_XMLNODE *xmlNodeSchema;
  GWEN_DB_NODE *dbData;
  int rv;

  assert(ie);
  ieh=GWEN_INHERIT_GETDATA(AB_IMEXPORTER, AB_IMEXPORTER_XML, ie);
  assert(ieh);

  dbSubParams=GWEN_DB_GetGroup(dbParams, GWEN_PATH_FLAGS_NAMEMUSTEXIST, "params");
  if (!dbSubParams) {
    DBG_ERROR(AQBANKING_LOGDOMAIN, "Missing \"params\" section in profile");
    return GWEN_ERROR_INVALID;
  }

  schemaName=GWEN_DB_GetCharValue(dbSubParams, "schema", 0, NULL);
  if (!(schemaName && *schemaName)) {
    DBG_ERROR(AQBANKING_LOGDOMAIN, "Missing schema name in parameters");
    return GWEN_ERROR_INVALID;
  }

  xmlDocSchema=AB_ImExporterXML_ReadSchemaFile(ie, schemaName);
  if (xmlDocSchema==NULL) {
    DBG_ERROR(AQBANKING_LOGDOMAIN, "Could not load schema file for \"%s\"", schemaName);
    return GWEN_ERROR_INVALID;
  }

  xmlDocData=AB_ImExporterXML_ReadXmlFromSio(ie, sio);
  if (xmlDocData==NULL) {
    DBG_ERROR(AQBANKING_LOGDOMAIN, "Could not read XML input");
    GWEN_XMLNode_free(xmlDocSchema);
    return GWEN_ERROR_INVALID;
  }

  xmlNodeSchema=GWEN_XMLNode_FindFirstTag(xmlDocSchema, "ImportSchema", NULL, NULL);
  if (!xmlNodeSchema) {
    DBG_ERROR(AQBANKING_LOGDOMAIN, "Missing \"ImportSchema\" in schema file.");
    GWEN_XMLNode_free(xmlDocData);
    GWEN_XMLNode_free(xmlDocSchema);
    return GWEN_ERROR_INVALID;
  }

  dbData=GWEN_DB_Group_new("data");
  rv=AB_ImExporterXML_ImportToDb(xmlDocData, xmlNodeSchema, dbData);

  DBG_ERROR(AQBANKING_LOGDOMAIN, "Data received:");
  GWEN_DB_Dump(dbData, 2);
  if (rv<0) {
    DBG_INFO(AQBANKING_LOGDOMAIN, "here (%d)", rv);
    GWEN_DB_Group_free(dbData);
    GWEN_XMLNode_free(xmlDocData);
    GWEN_XMLNode_free(xmlDocSchema);
    return rv;
  }

  /* TODO: import */

  GWEN_DB_Group_free(dbData);
  GWEN_XMLNode_free(xmlDocData);
  GWEN_XMLNode_free(xmlDocSchema);

  return 0;
}



int AB_ImExporterXML_Export(AB_IMEXPORTER *ie,
                              AB_IMEXPORTER_CONTEXT *ctx,
			      GWEN_SYNCIO *sio,
			      GWEN_DB_NODE *params){
  AB_IMEXPORTER_XML *ieh;

  assert(ie);
  ieh=GWEN_INHERIT_GETDATA(AB_IMEXPORTER, AB_IMEXPORTER_XML, ie);
  assert(ieh);

  return 0;
}



int AB_ImExporterXML_CheckFile(AB_IMEXPORTER *ie, const char *fname){
  AB_IMEXPORTER_XML *ieh;

  assert(ie);
  ieh=GWEN_INHERIT_GETDATA(AB_IMEXPORTER, AB_IMEXPORTER_XML, ie);
  assert(ieh);

  return 0;
}




GWEN_XMLNODE *AB_ImExporterXML_ReadSchemaFile(AB_IMEXPORTER *ie, const char *schemaName){
  GWEN_BUFFER *tbuf;
  GWEN_BUFFER *fullPathBuffer;
  GWEN_XMLNODE *xmlNode;
  int rv;

  fullPathBuffer=GWEN_Buffer_new(0, 256, 0, 1);

  tbuf=GWEN_Buffer_new(0, 256, 0, 1);
  GWEN_Buffer_AppendString(tbuf, schemaName);
  GWEN_Buffer_AppendString(tbuf, ".xml");

  rv=AB_Banking_FindDataFileForImExporter(AB_ImExporter_GetBanking(ie), "xml", GWEN_Buffer_GetStart(tbuf), fullPathBuffer);
  if (rv<0) {
    DBG_INFO(AQBANKING_LOGDOMAIN, "here (%d)", rv);
    GWEN_Buffer_free(tbuf);
    GWEN_Buffer_free(fullPathBuffer);
    return NULL;
  }
  GWEN_Buffer_free(tbuf);

  xmlNode=GWEN_XMLNode_new(GWEN_XMLNodeTypeTag, "schemaFile");
  rv=GWEN_XML_ReadFile(xmlNode, GWEN_Buffer_GetStart(fullPathBuffer),
                       GWEN_XML_FLAGS_HANDLE_COMMENTS | GWEN_XML_FLAGS_HANDLE_HEADERS);
  if (rv<0) {
    DBG_INFO(AQBANKING_LOGDOMAIN, "here (%d)", rv);
    GWEN_XMLNode_free(xmlNode);
    GWEN_Buffer_free(fullPathBuffer);
    return NULL;
  }

  GWEN_Buffer_free(fullPathBuffer);
  return xmlNode;
}



GWEN_XMLNODE *AB_ImExporterXML_ReadXmlFromSio(AB_IMEXPORTER *ie, GWEN_SYNCIO *sio){
  int rv;
  GWEN_XMLNODE *xmlDocRoot;
  GWEN_XML_CONTEXT *xmlCtx;

  /* read whole document into XML tree */
  xmlDocRoot=GWEN_XMLNode_new(GWEN_XMLNodeTypeTag, "xmlDocRoot");
  xmlCtx=GWEN_XmlCtxStore_new(xmlDocRoot, GWEN_XML_FLAGS_HANDLE_COMMENTS | GWEN_XML_FLAGS_HANDLE_HEADERS);
  rv=GWEN_XMLContext_ReadFromIo(xmlCtx, sio);
  if (rv<0) {
    DBG_INFO(AQBANKING_LOGDOMAIN, "here (%d)", rv);
    GWEN_XmlCtx_free(xmlCtx);
    GWEN_XMLNode_free(xmlDocRoot);
    return NULL;
  }
  GWEN_XmlCtx_free(xmlCtx);

  return xmlDocRoot;
}



#include "xml_import.c"




