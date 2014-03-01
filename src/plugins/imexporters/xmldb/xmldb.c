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

#include "xmldb_p.h"
#include "i18n_l.h"

#include <aqbanking/banking.h>
#include <aqbanking/accstatus.h>

#include <gwenhywfar/debug.h>
#include <gwenhywfar/misc.h>
#include <gwenhywfar/gui.h>
#include <gwenhywfar/inherit.h>



GWEN_INHERIT(AB_IMEXPORTER, AH_IMEXPORTER_XMLDB);



GWEN_PLUGIN *imexporter_xmldb_factory(GWEN_PLUGIN_MANAGER *pm,
				      const char *name,
				      const char *fileName) {
  GWEN_PLUGIN *pl;

  pl=AB_Plugin_ImExporter_new(pm, name, fileName);
  assert(pl);

  AB_Plugin_ImExporter_SetFactoryFn(pl, AB_Plugin_ImExporterXMLDB_Factory);

  return pl;
}



AB_IMEXPORTER *AB_Plugin_ImExporterXMLDB_Factory(GWEN_PLUGIN *pl,
						 AB_BANKING *ab){
  AB_IMEXPORTER *ie;
  AH_IMEXPORTER_XMLDB *ieh;

  ie=AB_ImExporter_new(ab, "xmldb");
  GWEN_NEW_OBJECT(AH_IMEXPORTER_XMLDB, ieh);
  GWEN_INHERIT_SETDATA(AB_IMEXPORTER, AH_IMEXPORTER_XMLDB, ie, ieh,
                       AH_ImExporterXMLDB_FreeData);
  ieh->dbio=GWEN_DBIO_GetPlugin("xmldb");
  if (!ieh->dbio) {
    DBG_ERROR(AQBANKING_LOGDOMAIN,
              "GWEN DBIO plugin \"XMLDB\" not available");
    AB_ImExporter_free(ie);
    return 0;
  }

  AB_ImExporter_SetImportFn(ie, AH_ImExporterXMLDB_Import);
  AB_ImExporter_SetExportFn(ie, AH_ImExporterXMLDB_Export);
  AB_ImExporter_SetCheckFileFn(ie, AH_ImExporterXMLDB_CheckFile);
  return ie;
}



void GWENHYWFAR_CB AH_ImExporterXMLDB_FreeData(void *bp, void *p){
  AH_IMEXPORTER_XMLDB *ieh;

  ieh=(AH_IMEXPORTER_XMLDB*)p;
  GWEN_DBIO_free(ieh->dbio);
  GWEN_FREE_OBJECT(ieh);
}



int AH_ImExporterXMLDB_Import(AB_IMEXPORTER *ie,
                              AB_IMEXPORTER_CONTEXT *ctx,
                              GWEN_SYNCIO *sio,
			      GWEN_DB_NODE *params){
  AH_IMEXPORTER_XMLDB *ieh;
  GWEN_DB_NODE *dbData;
  GWEN_DB_NODE *dbSubParams;
  int rv;

  assert(ie);
  ieh=GWEN_INHERIT_GETDATA(AB_IMEXPORTER, AH_IMEXPORTER_XMLDB, ie);
  assert(ieh);
  assert(ieh->dbio);

  dbSubParams=GWEN_DB_GetGroup(params, GWEN_PATH_FLAGS_NAMEMUSTEXIST,
			       "params");
  dbData=GWEN_DB_Group_new("transactions");
  GWEN_Gui_ProgressLog(0, GWEN_LoggerLevel_Notice,
		       I18N("Reading file..."));

  rv=GWEN_DBIO_Import(ieh->dbio,
		      sio,
                      dbData,
		      dbSubParams,
		      GWEN_DB_FLAGS_DEFAULT |
		      GWEN_PATH_FLAGS_CREATE_GROUP);
  if (rv) {
    DBG_ERROR(AQBANKING_LOGDOMAIN, "Error importing data");
    GWEN_Gui_ProgressLog(0, GWEN_LoggerLevel_Error,
			 I18N("Error importing data"));
    GWEN_DB_Group_free(dbData);
    return GWEN_ERROR_BAD_DATA;
  }

  /* transform DB to transactions */
  GWEN_Gui_ProgressLog(0, GWEN_LoggerLevel_Notice,
		       "Data imported, transforming to transactions");
  rv=AB_ImExporterContext_ReadDb(ctx, dbData);
  if (rv) {
    GWEN_Gui_ProgressLog(0, GWEN_LoggerLevel_Error,
			 "Error importing data");
    GWEN_DB_Group_free(dbData);
    return rv;
  }

  GWEN_DB_Group_free(dbData);
  return 0;
}



int AH_ImExporterXMLDB_Export(AB_IMEXPORTER *ie,
                              AB_IMEXPORTER_CONTEXT *ctx,
			      GWEN_SYNCIO *sio,
			      GWEN_DB_NODE *params){
  AH_IMEXPORTER_XMLDB *ieh;
  GWEN_DB_NODE *dbSubParams;
  GWEN_DB_NODE *dbData;
  int rv;

  assert(ie);
  ieh=GWEN_INHERIT_GETDATA(AB_IMEXPORTER, AH_IMEXPORTER_XMLDB, ie);
  assert(ieh);
  assert(ieh->dbio);

  dbSubParams=GWEN_DB_GetGroup(params, GWEN_PATH_FLAGS_NAMEMUSTEXIST,
                               "params");

  dbData=GWEN_DB_Group_new("GWEN_DB");
  rv=AB_ImExporterContext_toDb(ctx, dbData);
  if (rv) {
    DBG_ERROR(AQBANKING_LOGDOMAIN, "Error exporting data");
    GWEN_Gui_ProgressLog(0, GWEN_LoggerLevel_Error,
                          I18N("Error exporting data"));
    GWEN_DB_Group_free(dbData);
    return rv;
  }

  rv=GWEN_DBIO_Export(ieh->dbio,
		      sio,
                      dbData,
		      dbSubParams,
		      GWEN_DB_FLAGS_DEFAULT |
		      GWEN_PATH_FLAGS_CREATE_GROUP);
  if (rv) {
    DBG_ERROR(AQBANKING_LOGDOMAIN, "Error exporting data");
    GWEN_Gui_ProgressLog(0, GWEN_LoggerLevel_Error,
                          I18N("Error exporting data"));
    GWEN_DB_Group_free(dbData);
    return GWEN_ERROR_GENERIC;
  }

  GWEN_DB_Group_free(dbData);
  return 0;
}



int AH_ImExporterXMLDB_CheckFile(AB_IMEXPORTER *ie, const char *fname){
  AH_IMEXPORTER_XMLDB *ieh;
  GWEN_DBIO_CHECKFILE_RESULT rv;

  assert(ie);
  ieh=GWEN_INHERIT_GETDATA(AB_IMEXPORTER, AH_IMEXPORTER_XMLDB, ie);
  assert(ieh);
  assert(ieh->dbio);

  rv=GWEN_DBIO_CheckFile(ieh->dbio, fname);
  switch(rv) {
  case GWEN_DBIO_CheckFileResultOk:      return 0;
  case GWEN_DBIO_CheckFileResultNotOk:   return GWEN_ERROR_BAD_DATA;
  case GWEN_DBIO_CheckFileResultUnknown: return AB_ERROR_INDIFFERENT;
  default:                               return GWEN_ERROR_GENERIC;
  } /* switch */
}







