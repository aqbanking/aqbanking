/***************************************************************************
 $RCSfile$
 -------------------
 cvs         : $Id$
 begin       : Mon Mar 01 2004
 copyright   : (C) 2004 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

/** @file imexporter_be.h
 * @short This file is used by provider/importer/exporter plugins.
 */


#ifndef AQBANKING_IMEXPORTER_BE_H
#define AQBANKING_IMEXPORTER_BE_H


#include <aqbanking/imexporter.h>
#include <gwenhywfar/misc.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef AB_IMEXPORTER* (*AB_IMEXPORTER_FACTORY_FN)(AB_BANKING *ab,
                                                   GWEN_DB_NODE *db);


/** @name Construction and Destruction
 *
 */
/*@{*/
AQBANKING_API 
AB_IMEXPORTER *AB_ImExporter_new(AB_BANKING *ab,
                                 const char *name);
AQBANKING_API 
void AB_ImExporter_free(AB_IMEXPORTER *ie);
/*@}*/



/** @name Prototypes for Virtual Backend Functions
 *
 */
/*@{*/
typedef int (*AB_IMEXPORTER_IMPORT_FN)(AB_IMEXPORTER *ie,
                                       AB_IMEXPORTER_CONTEXT *ctx,
                                       GWEN_BUFFEREDIO *bio,
                                       GWEN_DB_NODE *params);

typedef int (*AB_IMEXPORTER_EXPORT_FN)(AB_IMEXPORTER *ie,
                                       AB_IMEXPORTER_CONTEXT *ctx,
                                       GWEN_BUFFEREDIO *bio,
                                       GWEN_DB_NODE *params);

/**
 * Checks whether the given file is possibly supported by the plugin.
 */
typedef int (*AB_IMEXPORTER_CHECKFILE_FN)(AB_IMEXPORTER *ie,
                                          const char *fname);


/*@}*/




/** @name Setters for Virtual Backend Functions
 *
 */
/*@{*/
AQBANKING_API 
void AB_ImExporter_SetImportFn(AB_IMEXPORTER *ie,
                               AB_IMEXPORTER_IMPORT_FN f);

AQBANKING_API 
void AB_ImExporter_SetExportFn(AB_IMEXPORTER *ie,
                               AB_IMEXPORTER_EXPORT_FN f);

AQBANKING_API
void AB_ImExporter_SetCheckFileFn(AB_IMEXPORTER *ie,
                                  AB_IMEXPORTER_CHECKFILE_FN f);

/*@}*/


#ifdef __cplusplus
}
#endif



#endif /* AQBANKING_IMEXPORTER_BE_H */


