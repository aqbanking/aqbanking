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



GWEN_INHERIT(AB_IMEXPORTER, AH_IMEXPORTER_SEPA);



GWEN_PLUGIN *imexporters_sepa_factory(GWEN_PLUGIN_MANAGER *pm,
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
                              GWEN_IO_LAYER *io,
			      GWEN_DB_NODE *params,
			      uint32_t guiid){
  AH_IMEXPORTER_SEPA *ieh;

  assert(ie);
  ieh=GWEN_INHERIT_GETDATA(AB_IMEXPORTER, AH_IMEXPORTER_SEPA, ie);
  assert(ieh);


  return GWEN_ERROR_NOT_SUPPORTED;
}



int AH_ImExporterSEPA_Export(AB_IMEXPORTER *ie,
			     AB_IMEXPORTER_CONTEXT *ctx,
			     GWEN_IO_LAYER *io,
			     GWEN_DB_NODE *params,
			     uint32_t guiid){
  AH_IMEXPORTER_SEPA *ieh;
  const char *s;

  assert(ie);
  ieh=GWEN_INHERIT_GETDATA(AB_IMEXPORTER, AH_IMEXPORTER_SEPA, ie);
  assert(ieh);

  s=GWEN_DB_GetCharValue(params, "type", 0, "ccm");
  if (strcasecmp(s, "ccm")==0) {
    return AH_ImExporterSEPA_Export_Ccm(ie, ctx, io, params, guiid);
  }
  else if (strcasecmp(s, "cdm")==0) {
  }
  else {
    DBG_ERROR(AQBANKING_LOGDOMAIN, "Unknown SEPA type \"%s\"", s);
    return GWEN_ERROR_INVALID;
  }

  /* TODO */
  return GWEN_ERROR_NOT_IMPLEMENTED;
}



int AH_ImExporterSEPA_CheckFile(AB_IMEXPORTER *ie, const char *fname, uint32_t guiid){
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





#include "sepa_exp_ccm.c"


