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
 * importer context.
 * </p>
 * <p>
 * The application can later browse through all transactions stored within the
 * given context and import them into its own database as needed.
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


/** @name Virtual Functions for Backends
 *
 */
/*@{*/

/**
 * Reads the given stream and imports all data from it. This imported
 * data is stored within the given context.
 * @param ie pointer to the importer/exporter
 * @param ctx import context
 * @param bio stream to read from (usually a file, see
 *   @ref GWEN_BufferedIO_File_new)
 * @param dbProfile configuration data for the importer. You can get this
 *   using @ref AB_Banking_GetImExporterProfiles.
 */
AQBANKING_API 
int AB_ImExporter_Import(AB_IMEXPORTER *ie,
                         AB_IMEXPORTER_CONTEXT *ctx,
                         GWEN_BUFFEREDIO *bio,
                         GWEN_DB_NODE *dbProfile);
/*@}*/



AQBANKING_API 
AB_BANKING *AB_ImExporter_GetBanking(const AB_IMEXPORTER *ie);
AQBANKING_API 
const char *AB_ImExporter_GetName(const AB_IMEXPORTER *ie);




/** @name Im-/export Context
 *
 * A context contains the list of imported accounts and a list of
 * transactions.
 */
/*@{*/
AQBANKING_API 
AB_IMEXPORTER_CONTEXT *AB_ImExporterContext_new();
AQBANKING_API 
void AB_ImExporterContext_free(AB_IMEXPORTER_CONTEXT *iec);

/**
 * Takes over ownership of the given account.
 */
AQBANKING_API 
void AB_ImExporterContext_AddAccount(AB_IMEXPORTER_CONTEXT *iec,
                                     AB_ACCOUNT *a);

/**
 * Returns the first imported account (if any).
 * The caller becomes the new owner of the account returned (if any),
 * so he/she is responsible for calling @ref AB_Account_free() when finished.
 */
AQBANKING_API 
AB_ACCOUNT*
  AB_ImExporterContext_GetFirstAccount(AB_IMEXPORTER_CONTEXT *iec);

/**
 * Returns the next imported account (if any).
 * The caller becomes the new owner of the account returned (if any),
 * so he/she is responsible for calling @ref AB_Account_free() when finished.
 */
AQBANKING_API 
AB_ACCOUNT*
  AB_ImExporterContext_GetNextAccount(AB_IMEXPORTER_CONTEXT *iec);


/**
 * Takes over ownership of the given transaction.
 */
AQBANKING_API 
void AB_ImExporterContext_AddTransaction(AB_IMEXPORTER_CONTEXT *iec,
                                         AB_TRANSACTION *t);

/**
 * Returns the first transaction stored within the context.
 * The caller becomes the new owner of the transaction returned (if any)
 * which makes him/her responsible for freeing it using
 * @ref AB_Transaction_free.
 */
AQBANKING_API 
AB_TRANSACTION*
AB_ImExporterContext_GetFirstTransaction(AB_IMEXPORTER_CONTEXT *iec);

/**
 * Returns the next transaction stored within the context.
 * The caller becomes the new owner of the transaction returned (if any)
 * which makes him/her responsible for freeing it using
 * @ref AB_Transaction_free.
 */
AQBANKING_API 
AB_TRANSACTION*
AB_ImExporterContext_GetNextTransaction(AB_IMEXPORTER_CONTEXT *iec);
/*@}*/


#ifdef __cplusplus
}
#endif


/*@}*/ /* defgroup */


#endif /* AQBANKING_IMEXPORTER_H */


