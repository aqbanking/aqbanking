/***************************************************************************
    begin       : Sat Dec 15 2018
    copyright   : (C) 2018 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif


#include "camt_p.h"


#include "i18n_l.h"
#include <aqbanking/banking.h>
#include <gwenhywfar/debug.h>
#include <gwenhywfar/misc.h>
#include <gwenhywfar/gui.h>
#include <gwenhywfar/inherit.h>
#include <gwenhywfar/gwendate.h>
#include <gwenhywfar/gwentime.h>
#include <gwenhywfar/text.h>

#include <ctype.h>




GWEN_INHERIT(AB_IMEXPORTER, AH_IMEXPORTER_CAMT);



AB_IMEXPORTER *AB_ImExporterCAMT_new(AB_BANKING *ab){
  AB_IMEXPORTER *ie;
  AH_IMEXPORTER_CAMT *ieh;

  ie=AB_ImExporter_new(ab, "camt");
  GWEN_NEW_OBJECT(AH_IMEXPORTER_CAMT, ieh);
  GWEN_INHERIT_SETDATA(AB_IMEXPORTER, AH_IMEXPORTER_CAMT, ie, ieh, AH_ImExporterCAMT_FreeData);

  AB_ImExporter_SetImportFn(ie, AH_ImExporterCAMT_Import);
  AB_ImExporter_SetExportFn(ie, AH_ImExporterCAMT_Export);
  AB_ImExporter_SetCheckFileFn(ie, AH_ImExporterCAMT_CheckFile);
  return ie;
}



void GWENHYWFAR_CB AH_ImExporterCAMT_FreeData(void *bp, void *p){
  AH_IMEXPORTER_CAMT *ieh;

  ieh=(AH_IMEXPORTER_CAMT*)p;
  GWEN_FREE_OBJECT(ieh);
}



int AH_ImExporterCAMT_Import(AB_IMEXPORTER *ie,
                             AB_IMEXPORTER_CONTEXT *ctx,
                             GWEN_SYNCIO *sio,
			     GWEN_DB_NODE *params) {
  int rv;
  GWEN_XMLNODE *xmlRoot;
  GWEN_XMLNODE *n;
  GWEN_XML_CONTEXT *xmlCtx;
  const char *camVersionWanted;

  /* read whole document into XML tree */
  xmlRoot=GWEN_XMLNode_new(GWEN_XMLNodeTypeTag, "camt52");
  xmlCtx=GWEN_XmlCtxStore_new(xmlRoot, GWEN_XML_FLAGS_DEFAULT);
  rv=GWEN_XMLContext_ReadFromIo(xmlCtx, sio);
  if (rv<0) {
    GWEN_XmlCtx_free(xmlCtx);
    GWEN_XMLNode_free(xmlRoot);
    return rv;
  }
  GWEN_XmlCtx_free(xmlCtx);

  n=GWEN_XMLNode_FindFirstTag(xmlRoot, "Document", NULL, NULL);
  if (n==NULL) {
    DBG_ERROR(AQBANKING_LOGDOMAIN, "<Document> element not found");
    GWEN_XMLNode_free(xmlRoot);
    return GWEN_ERROR_BAD_DATA;
  }

  /* debug */
  /*GWEN_XMLNode_Dump(n, 2); */

  /* check document type */
  camVersionWanted=GWEN_DB_GetCharValue(params, "type", 0, "052.001.02");
  assert(camVersionWanted);

  if (strcasecmp(camVersionWanted, "052.001.02")==0)
    rv=AH_ImExporterCAMT_Import_052_001_02(ie, ctx, params, n);
  else
    rv=0;

  GWEN_XMLNode_free(xmlRoot);
  return rv;
}



int AH_ImExporterCAMT_Export(AB_IMEXPORTER *ie,
                             AB_IMEXPORTER_CONTEXT *ctx,
                             GWEN_SYNCIO *sio,
                             GWEN_DB_NODE *params) {
  return GWEN_ERROR_NOT_SUPPORTED;
}



int AH_ImExporterCAMT_CheckFile(AB_IMEXPORTER *ie, const char *fname) {
  return 0;
}




#include "camt52_001_02.c"




