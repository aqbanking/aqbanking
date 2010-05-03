/***************************************************************************
    begin       : Mon Mar 01 2004
    copyright   : (C) 2004-2010 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "ctxfile_p.h"
#include "i18n_l.h"
#include <aqbanking/banking.h>
#include <gwenhywfar/debug.h>
#include <gwenhywfar/misc.h>
#include <gwenhywfar/gui.h>
#include <gwenhywfar/inherit.h>


GWEN_INHERIT(AB_IMEXPORTER, AH_IMEXPORTER_CTXFILE);



GWEN_PLUGIN *imexporter_ctxfile_factory(GWEN_PLUGIN_MANAGER *pm,
					const char *name,
					const char *fileName) {
  GWEN_PLUGIN *pl;

  pl=AB_Plugin_ImExporter_new(pm, name, fileName);
  assert(pl);

  AB_Plugin_ImExporter_SetFactoryFn(pl, AB_Plugin_ImExporterCtxFile_Factory);

  return pl;
}



AB_IMEXPORTER *AB_Plugin_ImExporterCtxFile_Factory(GWEN_PLUGIN *pl, AB_BANKING *ab){
  AB_IMEXPORTER *ie;
  AH_IMEXPORTER_CTXFILE *ieh;

  ie=AB_ImExporter_new(ab, "ctxfile");
  GWEN_NEW_OBJECT(AH_IMEXPORTER_CTXFILE, ieh);
  GWEN_INHERIT_SETDATA(AB_IMEXPORTER, AH_IMEXPORTER_CTXFILE, ie, ieh,
                       AH_ImExporterCtxFile_FreeData);

  AB_ImExporter_SetImportFn(ie, AH_ImExporterCtxFile_Import);
  AB_ImExporter_SetExportFn(ie, AH_ImExporterCtxFile_Export);
  AB_ImExporter_SetCheckFileFn(ie, AH_ImExporterCtxFile_CheckFile);
  return ie;
}



void GWENHYWFAR_CB AH_ImExporterCtxFile_FreeData(void *bp, void *p){
  AH_IMEXPORTER_CTXFILE *ieh;

  ieh=(AH_IMEXPORTER_CTXFILE*)p;
  GWEN_FREE_OBJECT(ieh);
}



int AH_ImExporterCtxFile_Import(AB_IMEXPORTER *ie,
				AB_IMEXPORTER_CONTEXT *ctx,
				GWEN_SYNCIO *sio,
				GWEN_DB_NODE *params){
  AH_IMEXPORTER_CTXFILE *ieh;
  GWEN_DB_NODE *dbData;
  int rv;

  assert(ie);
  ieh=GWEN_INHERIT_GETDATA(AB_IMEXPORTER, AH_IMEXPORTER_CTXFILE, ie);
  assert(ieh);

  dbData=GWEN_DB_Group_new("context");
  rv=GWEN_DB_ReadFromIo(dbData,
			sio,
			GWEN_DB_FLAGS_DEFAULT |
			GWEN_PATH_FLAGS_CREATE_GROUP);
  if (rv) {
    DBG_ERROR(AQBANKING_LOGDOMAIN, "Error importing data (%d)", rv);
    GWEN_Gui_ProgressLog(0, GWEN_LoggerLevel_Error,
			 "Error importing data");
    GWEN_DB_Group_free(dbData);
    return GWEN_ERROR_GENERIC;
  }

  /* transform DB to transactions */
  GWEN_Gui_ProgressLog(0, GWEN_LoggerLevel_Notice,
		       I18N("Data imported, transforming to UTF-8"));
  rv=AB_ImExporter_DbFromIso8859_1ToUtf8(dbData);
  if (rv) {
    GWEN_Gui_ProgressLog(0, GWEN_LoggerLevel_Error,
			 "Error converting data");
    GWEN_DB_Group_free(dbData);
    return rv;
  }
  GWEN_Gui_ProgressLog(0, GWEN_LoggerLevel_Notice,
		       "Transforming data to transactions");

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



int AH_ImExporterCtxFile_CheckFile(AB_IMEXPORTER *ie, const char *fname){
  AH_IMEXPORTER_CTXFILE *ieh;

  assert(ie);
  ieh=GWEN_INHERIT_GETDATA(AB_IMEXPORTER, AH_IMEXPORTER_CTXFILE, ie);
  assert(ieh);

  /* always return indifferent (for now) */
  return AB_ERROR_INDIFFERENT;
}



int AH_ImExporterCtxFile_Export(AB_IMEXPORTER *ie,
				AB_IMEXPORTER_CONTEXT *ctx,
				GWEN_SYNCIO *sio,
				GWEN_DB_NODE *params){
  AH_IMEXPORTER_CTXFILE *ieh;
  GWEN_DB_NODE *dbData;
  int rv;

  assert(ie);
  ieh=GWEN_INHERIT_GETDATA(AB_IMEXPORTER, AH_IMEXPORTER_CTXFILE, ie);
  assert(ieh);

  /* create db, store context in it */
  dbData=GWEN_DB_Group_new("context");

  rv=AB_ImExporterContext_toDb(ctx, dbData);
  if (rv) {
    DBG_ERROR(AQBANKING_LOGDOMAIN, "Error exporting data (%d)", rv);
    GWEN_Gui_ProgressLog(0, GWEN_LoggerLevel_Error,
			 "Error exporting data");
    GWEN_DB_Group_free(dbData);
    return GWEN_ERROR_GENERIC;
  }

  rv=GWEN_DB_WriteToIo(dbData, sio, GWEN_DB_FLAGS_DEFAULT);
  if (rv) {
    DBG_ERROR(AQBANKING_LOGDOMAIN, "Error exporting data (%d)", rv);
    GWEN_Gui_ProgressLog(0, GWEN_LoggerLevel_Error,
			 "Error exporting data");
    GWEN_DB_Group_free(dbData);
    return GWEN_ERROR_GENERIC;
  }
  GWEN_DB_Group_free(dbData);

  return 0;
}




