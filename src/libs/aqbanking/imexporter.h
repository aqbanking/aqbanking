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


#ifndef AQBANKING_IMEXPORTER_H
#define AQBANKING_IMEXPORTER_H

#include <gwenhywfar/inherit.h>
#include <gwenhywfar/bufferedio.h>
#include <gwenhywfar/db.h>
#include <gwenhywfar/types.h>
#include <aqbanking/error.h>


/** @defgroup AB_IMEXPORTER Generic Im- and Exporter
 * @short Generic Financial Data Importer/Exporter
 * <p>
 * This group contains a generic importer/exporter.
 * </p>
 * <h2>Importing</h2>
 * <p>
 * When importing this group reads transactions and accounts from a
 * given stream (in most cases a file) and stores them in a given
 * importer context. Transactions are automatically sorted by date (or
 * valuta date if the date is not given in the transactions read).
 * </p>
 * <p>
 * The application can later browse through all days stored within the given
 * context and import the transactions stored for those days into its own
 * database as needed.
 * </p>
 */
/*@{*/

#ifdef __cplusplus
extern "C" {
#endif

typedef struct AB_IMEXPORTER AB_IMEXPORTER;
GWEN_INHERIT_FUNCTION_LIB_DEFS(AB_IMEXPORTER, AQBANKING_API)

typedef struct AB_IMEXPORTER_DAY AB_IMEXPORTER_DAY;
typedef struct AB_IMEXPORTER_CONTEXT AB_IMEXPORTER_CONTEXT;

#ifdef __cplusplus
}
#endif


#include <aqbanking/banking.h>
#include <aqbanking/account.h>
#include <aqbanking/transaction.h>


#ifdef __cplusplus
extern "C" {
#endif


/** @name Prototypes for Virtual Backend Functions
 *
 */
/*@{*/
typedef int (*AB_IMEXPORTER_IMPORT_FN)(AB_IMEXPORTER *ie,
                                       AB_IMEXPORTER_CONTEXT *ctx,
                                       GWEN_BUFFEREDIO *bio,
                                       GWEN_DB_NODE *params);
/*@}*/




/** @name Construction and Destruction
 *
 */
/*@{*/
AB_IMEXPORTER *AB_ImExporter_new(AB_BANKING *ab,
                                 const char *name);

void AB_ImExporter_free(AB_IMEXPORTER *ie);
/*@}*/



/** @name Virtual Functions for Backends
 *
 */
/*@{*/

/**
 * Reads the given stream and imports all data from it. This imported
 * data is stored within the given context.
 */
int AB_ImExporter_Import(AB_IMEXPORTER *ie,
                         AB_IMEXPORTER_CONTEXT *ctx,
                         GWEN_BUFFEREDIO *bio,
                         GWEN_DB_NODE *params);
/*@}*/



/** @name Setters for Virtual Backend Functions
 *
 */
/*@{*/
void AB_ImExporter_SetImportFn(AB_IMEXPORTER *ie,
                               AB_IMEXPORTER_IMPORT_FN f);
/*@}*/



AB_BANKING *AB_ImExporter_GetBanking(const AB_IMEXPORTER *ie);
const char *AB_ImExporter_GetName(const AB_IMEXPORTER *ie);




/** @name Im-/export Context
 *
 * A context contains the list of imported accounts and a list of days
 * for which transactions have been imported.
 */
/*@{*/
AB_IMEXPORTER_CONTEXT *AB_ImExporterContext_new();
void AB_ImExporterContext_free(AB_IMEXPORTER_CONTEXT *iec);

/**
 * Returns the first day for which transactions are available.
 * The importer keeps ownership of the object returned (if any), so you
 * <b>must not</b> free it.
 * Please note that the days are not necessarily sorted.
 */
AB_IMEXPORTER_DAY*
  AB_ImExporterContext_GetFirstDay(AB_IMEXPORTER_CONTEXT *iec);

/**
 * Returns the next day for which transactions are available.
 * The importer keeps ownership of the object returned (if any), so you
 * <b>must not</b> free it.
 * Please note that the days are not necessarily sorted.
 */
AB_IMEXPORTER_DAY*
  AB_ImExporterContext_GetNextDay(AB_IMEXPORTER_CONTEXT *iec);

/**
 * Returns the first imported account (if any).
 * The caller becomes the new owner of the account returned (if any),
 * so he/she is responsible for calling AB_Account_free() when finished.
 */
AB_ACCOUNT*
  AB_ImExporterContext_GetFirstAccount(AB_IMEXPORTER_CONTEXT *iec);

/**
 * Returns the next imported account (if any).
 * The caller becomes the new owner of the account returned (if any),
 * so he/she is responsible for calling AB_Account_free() when finished.
 */
AB_ACCOUNT*
  AB_ImExporterContext_GetNextAccount(AB_IMEXPORTER_CONTEXT *iec);
/*@}*/




/** @name Im-/export Day
 *
 */
/*@{*/
const GWEN_TIME *AB_ImExporterDay_GetDate(const AB_IMEXPORTER_DAY *ied);

/**
 * Returns the first transaction of the given day.
 * The caller becomes the new owner of the transaction returned (if any),
 * so he/she is responsible for calling AB_Transaction_free() when finished.
 */
AB_TRANSACTION *AB_ImExporterDay_GetFirstTransaction(AB_IMEXPORTER_DAY *ied);

/**
 * Returns the next transaction of the given day.
 * The caller becomes the new owner of the transaction returned (if any),
 * so he/she is responsible for calling AB_Transaction_free() when finished.
 */
AB_TRANSACTION *AB_ImExporterDay_GetNextTransaction(AB_IMEXPORTER_DAY *ied);
/*@}*/


#ifdef __cplusplus
}
#endif


/*@}*/ /* defgroup */


#endif /* AQBANKING_IMEXPORTER_H */


