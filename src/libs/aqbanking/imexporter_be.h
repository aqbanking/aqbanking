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
AB_IMEXPORTER *AB_ImExporter_new(AB_BANKING *ab,
                                 const char *name);
/*@}*/



/** @name Prototypes for Virtual Backend Functions
 *
 */
/*@{*/
typedef int (*AB_IMEXPORTER_IMPORT_FN)(AB_IMEXPORTER *ie,
                                       AB_IMEXPORTER_CONTEXT *ctx,
                                       GWEN_BUFFEREDIO *bio,
                                       GWEN_DB_NODE *params);
/*@}*/



/**
 * Takes over ownership of the given transaction.
 */
void AB_ImExporterContext_AddTransaction(AB_IMEXPORTER_CONTEXT *iec,
                                         AB_TRANSACTION *t);


/**
 * Takes over ownership of the given account.
 */
void AB_ImExporterContext_AddAccount(AB_IMEXPORTER_CONTEXT *iec,
                                     AB_ACCOUNT *a);


/** @name Setters for Virtual Backend Functions
 *
 */
/*@{*/
void AB_ImExporter_SetImportFn(AB_IMEXPORTER *ie,
                               AB_IMEXPORTER_IMPORT_FN f);
/*@}*/


#ifdef __cplusplus
}
#endif



#endif /* AQBANKING_IMEXPORTER_BE_H */


