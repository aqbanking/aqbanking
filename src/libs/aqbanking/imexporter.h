/***************************************************************************
 begin       : Mon Mar 01 2004
 copyright   : (C) 2018 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 * This file is part of the project "AqBanking".                           *
 * Please see toplevel file COPYING of that project for license details.   *
 ***************************************************************************/


#ifndef AQBANKING_IMEXPORTER_H
#define AQBANKING_IMEXPORTER_H

#include <gwenhywfar/inherit.h>
#include <gwenhywfar/syncio.h>
#include <gwenhywfar/db.h>
#include <gwenhywfar/types.h>
#include <gwenhywfar/dialog.h>

#include <aqbanking/error.h>


/** @addtogroup G_AB_IMEXPORTER Generic Im- and Exporter
 *
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



/** @name Flags returned by @ref AB_ImExporter_GetFlags
 *
 */
/*@{*/

/** This module supports the function @ref AB_ImExporter_GetEditProfileDialog */
#define AB_IMEXPORTER_FLAGS_GETPROFILEEDITOR_SUPPORTED 0x00000001


/*@}*/



#ifdef __cplusplus
extern "C" {
#endif

typedef struct AB_IMEXPORTER AB_IMEXPORTER;
GWEN_INHERIT_FUNCTION_LIB_DEFS(AB_IMEXPORTER, AQBANKING_API)

#ifdef __cplusplus
}
#endif


#include <aqbanking/imexporter_context.h>
#include <aqbanking/imexporter_accountinfo.h>
#include <aqbanking/banking.h>
#include <aqbanking/account.h>


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
			 GWEN_SYNCIO *sio,
			 GWEN_DB_NODE *dbProfile);

/**
 * Writes all data to the given stream.
 * @param ie pointer to the importer/exporter
 * @param ctx export context
 * @param bio stream to write to (usually a file, see
 *   @ref GWEN_BufferedIO_File_new)
 * @param dbProfile configuration data for the exporter. You can get this
 *   using @ref AB_Banking_GetImExporterProfiles.
 */
AQBANKING_API 
int AB_ImExporter_Export(AB_IMEXPORTER *ie,
                         AB_IMEXPORTER_CONTEXT *ctx,
			 GWEN_SYNCIO *sio,
			 GWEN_DB_NODE *dbProfile);

/**
 * This function should return a dialog (see @ref GWEN_DIALOG) which
 * allows editing of the given profile.
 * You can use @ref AB_ImExporter_GetFlags to determine beforehand whether
 * this function is supported (look for
 * @ref AB_IMEXPORTER_FLAGS_GETPROFILEEDITOR_SUPPORTED).
 * (introduced in AqBanking 4.3.0)
 *
 * @param ie pointer to the importer/exporter
 *
 * @param dbProfile configuration data for the exporter. You can get this
 *   using @ref AB_Banking_GetImExporterProfiles.
 *
 * @param pDlg pointer to a dialog pointer (receives the created dialog if any)
 *
 * @return 0 on success, error code otherwise
 */
AQBANKING_API
int AB_ImExporter_GetEditProfileDialog(AB_IMEXPORTER *ie,
				       GWEN_DB_NODE *dbProfile,
				       const char *testFileName,
				       GWEN_DIALOG **pDlg);


/**
 * This is just a convenience function for @ref AB_ImExporter_Import.
 */
AQBANKING_API
int AB_ImExporter_ImportFile(AB_IMEXPORTER *ie,
                             AB_IMEXPORTER_CONTEXT *ctx,
                             const char *fname,
			     GWEN_DB_NODE *dbProfile);

AQBANKING_API
int AB_ImExporter_ImportBuffer(AB_IMEXPORTER *ie,
			       AB_IMEXPORTER_CONTEXT *ctx,
                               GWEN_BUFFER *buf,
			       GWEN_DB_NODE *dbProfile);

AQBANKING_API
int AB_ImExporter_ExportToBuffer(AB_IMEXPORTER *ie,
				 AB_IMEXPORTER_CONTEXT *ctx,
				 GWEN_BUFFER *buf,
				 GWEN_DB_NODE *dbProfile);

AQBANKING_API
int AB_ImExporter_ExportToFile(AB_IMEXPORTER *ie,
			       AB_IMEXPORTER_CONTEXT *ctx,
			       const char *fname,
			       GWEN_DB_NODE *dbProfile);

/**
 * This function checks whether the given importer supports the given file.
 */
AQBANKING_API
int AB_ImExporter_CheckFile(AB_IMEXPORTER *ie,
			    const char *fname);

/*@}*/


/**
 * Returns the AB_BANKING object to which the im/exporter belongs.
 */
AQBANKING_API 
AB_BANKING *AB_ImExporter_GetBanking(const AB_IMEXPORTER *ie);

/**
 * Returns the name of the im/exporter.
 */
AQBANKING_API
const char *AB_ImExporter_GetName(const AB_IMEXPORTER *ie);


/**
 * Returns the flags if this im/exporter which specify the supported
 * features.
 */
AQBANKING_API
uint32_t AB_ImExporter_GetFlags(const AB_IMEXPORTER *ie);


/*@}*/ /* defgroup */





/** @name Helper Functions
 *
 * These functions are most likely used by implementations of im/exporters.
 */
/*@{*/
/**
 * Transforms an UTF-8 string to a DTA string. Untranslateable characters
 * are replaced by a space (chr 32).
 */
AQBANKING_API
void AB_ImExporter_Utf8ToDta(const char *p, int size, GWEN_BUFFER *buf);

/**
 * Transforms a DTA string to an UTF-8 string.
 */
AQBANKING_API 
void AB_ImExporter_DtaToUtf8(const char *p, int size, GWEN_BUFFER *buf);

AQBANKING_API 
void AB_ImExporter_Iso8859_1ToUtf8(const char *p, int size, GWEN_BUFFER *buf);

AQBANKING_DEPRECATED AQBANKING_API
int AH_ImExporter_DbFromIso8859_1ToUtf8(GWEN_DB_NODE *db);

/**
 * This function call @ref AB_ImExporter_Iso8859_1ToUtf8 on all char
 * values in the given db.
 */
AQBANKING_API 
int AB_ImExporter_DbFromIso8859_1ToUtf8(GWEN_DB_NODE *db);

AQBANKING_API 
GWEN_TIME *AB_ImExporter_DateFromString(const char *p, const char *tmpl, int inUtc);


/*@}*/



#ifdef __cplusplus
}
#endif




#endif /* AQBANKING_IMEXPORTER_H */


