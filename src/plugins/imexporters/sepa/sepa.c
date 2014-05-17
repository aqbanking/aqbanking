/***************************************************************************
    begin       : Mon Mar 01 2004
    copyright   : (C) 2004 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "sepa_p.h"
#include "i18n_l.h"

#include <aqbanking/banking.h>
#include <aqbanking/accstatus.h>

#include <gwenhywfar/debug.h>
#include <gwenhywfar/misc.h>
#include <gwenhywfar/gui.h>
#include <gwenhywfar/inherit.h>

#include <stdlib.h>
#include <ctype.h>



GWEN_INHERIT(AB_IMEXPORTER, AH_IMEXPORTER_SEPA);



GWEN_PLUGIN *imexporter_sepa_factory(GWEN_PLUGIN_MANAGER *pm,
				     const char *name,
				     const char *fileName) {
  GWEN_PLUGIN *pl;

  pl=AB_Plugin_ImExporter_new(pm, name, fileName);
  assert(pl);

  AB_Plugin_ImExporter_SetFactoryFn(pl, AB_Plugin_ImExporterSEPA_Factory);

  return pl;
}



AB_IMEXPORTER *AB_Plugin_ImExporterSEPA_Factory(GWEN_PLUGIN *pl,
						AB_BANKING *ab){
  AB_IMEXPORTER *ie;
  AH_IMEXPORTER_SEPA *ieh;

  ie=AB_ImExporter_new(ab, "sepa");
  GWEN_NEW_OBJECT(AH_IMEXPORTER_SEPA, ieh);
  GWEN_INHERIT_SETDATA(AB_IMEXPORTER, AH_IMEXPORTER_SEPA, ie, ieh,
		       AH_ImExporterSEPA_FreeData);
  AB_ImExporter_SetImportFn(ie, AH_ImExporterSEPA_Import);
  AB_ImExporter_SetExportFn(ie, AH_ImExporterSEPA_Export);
  AB_ImExporter_SetCheckFileFn(ie, AH_ImExporterSEPA_CheckFile);
  return ie;
}



void GWENHYWFAR_CB AH_ImExporterSEPA_FreeData(void *bp, void *p){
  AH_IMEXPORTER_SEPA *ieh;

  ieh=(AH_IMEXPORTER_SEPA*)p;
  GWEN_FREE_OBJECT(ieh);
}



int AH_ImExporterSEPA_Import(AB_IMEXPORTER *ie,
			     AB_IMEXPORTER_CONTEXT *ctx,
			     GWEN_SYNCIO *sio,
			     GWEN_DB_NODE *params){
  AH_IMEXPORTER_SEPA *ieh;

  assert(ie);
  ieh=GWEN_INHERIT_GETDATA(AB_IMEXPORTER, AH_IMEXPORTER_SEPA, ie);
  assert(ieh);


  return GWEN_ERROR_NOT_SUPPORTED;
}



int AH_ImExporterSEPA_Export(AB_IMEXPORTER *ie,
			     AB_IMEXPORTER_CONTEXT *ctx,
			     GWEN_SYNCIO *sio,
			     GWEN_DB_NODE *params){
  AH_IMEXPORTER_SEPA *ieh;
  GWEN_XMLNODE *root;
  GWEN_XMLNODE *documentNode;
  GWEN_XMLNODE *topNode;
  GWEN_XMLNODE *n;
  uint32_t doctype[]={0, 0, 0};
  int (*ctxToXml)(AB_IMEXPORTER *ie,
		  AB_IMEXPORTER_CONTEXT *ctx,
		  GWEN_XMLNODE *topNode,
		  uint32_t doctype[],
		  GWEN_DB_NODE *params);
  const char *s;
  int rv;

  assert(ie);
  ieh=GWEN_INHERIT_GETDATA(AB_IMEXPORTER, AH_IMEXPORTER_SEPA, ie);
  assert(ieh);

  s=GWEN_DB_GetCharValue(params, "type", 0, 0);
  if (s) {
    int i, j;
    const char *p;
    char *tail;

    /* Parse strings of the form xxx.yyy.zz */
    p=tail=(char*)s;
    for (i=0; i<3 && *tail; i++) {
      j=strtol(p, &tail, 10);
      if (!isspace(*p) &&
	  ((*tail=='.' && tail-p==3) || (*tail=='\0' && tail-p==2)) &&
	  j>0)
	doctype[i]=j;
      else
	break;
      p=tail+1;
    }
    if (i<3)
      /* Parsing the "type" option failed, record it for later reference */
      doctype[0]=0;
  }

  root=GWEN_XMLNode_new(GWEN_XMLNodeTypeTag, "root");
  n=GWEN_XMLNode_new(GWEN_XMLNodeTypeTag, "?xml");
  if (n) {
    GWEN_XMLNode_AddHeader(root, n);
    GWEN_XMLNode_SetProperty(n, "version", "1.0");
    GWEN_XMLNode_SetProperty(n, "encoding", "UTF-8");
  }

  documentNode=GWEN_XMLNode_new(GWEN_XMLNodeTypeTag, "Document");
  s=GWEN_DB_GetCharValue(params, "xmlns", 0, 0);
  if (!s || !*s) {
    DBG_ERROR(AQBANKING_LOGDOMAIN,
	      "xmlns not specified in profile \"%s\"",
	      GWEN_DB_GetCharValue(params, "name", 0, 0));
    GWEN_XMLNode_free(root);
    return GWEN_ERROR_INVALID;
  }
  GWEN_XMLNode_SetProperty(documentNode, "xmlns", s);
  GWEN_XMLNode_AddChild(root, documentNode);

  switch(doctype[0]) {
  case 1:
    if (doctype[1]>1 || doctype[2]>2)
      s="CstmrCdtTrfInitn";
    else
      s=strstr(s, "pain");
    ctxToXml=AH_ImExporterSEPA_Export_Pain_001;
    break;
  case 8:
    if (!(doctype[1]==1 && doctype[2]==1))
      s="CstmrDrctDbtInitn";
    else
      s=strstr(s, "pain");
    ctxToXml=AH_ImExporterSEPA_Export_Pain_008;
    break;
  default:
    DBG_ERROR(AQBANKING_LOGDOMAIN, "Unknown SEPA type \"%s\"",
	      GWEN_DB_GetCharValue(params, "type", 0, 0));
    GWEN_XMLNode_free(root);
    return GWEN_ERROR_INVALID;
  }

  topNode=GWEN_XMLNode_new(GWEN_XMLNodeTypeTag, s);
  GWEN_XMLNode_AddChild(documentNode, topNode);
  rv=ctxToXml(ie, ctx, topNode, doctype, params);
  if (rv==0) {
    GWEN_XML_CONTEXT *xmlctx;

    xmlctx=GWEN_XmlCtxStore_new(root,
				GWEN_XML_FLAGS_INDENT |
				GWEN_XML_FLAGS_SIMPLE |
				GWEN_XML_FLAGS_HANDLE_HEADERS);

    rv=GWEN_XMLNode_WriteToStream(root, xmlctx, sio);
    if (rv)
      DBG_INFO(AQBANKING_LOGDOMAIN, "here (%d)", rv);
    GWEN_XmlCtx_free(xmlctx);
    GWEN_XMLNode_free(root);
  }

  /* TODO */
  return rv;
}



int AH_ImExporterSEPA_CheckFile(AB_IMEXPORTER *ie, const char *fname){
  AH_IMEXPORTER_SEPA *ieh;

  assert(ie);
  ieh=GWEN_INHERIT_GETDATA(AB_IMEXPORTER, AH_IMEXPORTER_SEPA, ie);
  assert(ieh);

#if 0
  return AB_ERROR_INDIFFERENT;
#else
  /* TODO */
  return GWEN_ERROR_NOT_IMPLEMENTED;
#endif
}





#include "sepa_pain_001.c"
#include "sepa_pain_008.c"


