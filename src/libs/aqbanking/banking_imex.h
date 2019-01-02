/***************************************************************************
 begin       : Mon Mar 01 2004
 copyright   : (C) 2018 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 * This file is part of the project "AqBanking".                           *
 * Please see toplevel file COPYING of that project for license details.   *
 ***************************************************************************/


#ifndef AQBANKING_BANKING_IMEX_H
#define AQBANKING_BANKING_IMEX_H

#include <aqbanking/transaction.h>
#include <aqbanking/imexporter_context.h>
#include <aqbanking/imexporter_accountinfo.h>

#include <gwenhywfar/plugindescr.h>


#ifdef __cplusplus
extern "C" {
#endif


/** @addtogroup G_AB_IMEXPORTER
 */
/*@{*/


/** @name Plugin Handling
 *
 */
/*@{*/
/**
 * Returns a list2 of available importers and exporters.
 * You must free this list after using it via
 * @ref GWEN_PluginDescription_List2_freeAll.
 * Please note that a simple @ref GWEN_PluginDescription_List2_free would
 * not suffice, since that would only free the list but not the objects
 * stored within the list !
 * @param ab pointer to the AB_BANKING object
 */
AQBANKING_API
GWEN_PLUGIN_DESCRIPTION_LIST2 *AB_Banking_GetImExporterDescrs(AB_BANKING *ab);

/**
 * <p>
 * Loads all available profiles for the given importer/exporter.
 * This includes global profiles as well as local ones.
 * </p>
 * <p>
 * Local profiles overwrite global ones, allowing the user to customize the
 * profiles. Local profiles are expected in a folder below the user
 * local folder (e.g. "$HOME/.aqbanking"). The local profile folder for the
 * CSV plugin is in "$HOME/.aqbanking/imexporters/csv/profiles".
 * </p>
 * <p>
 * The GWEN_DB returned contains one group for every loaded profile. Every
 * group has the name of the profile it contains. Every group contains at
 * least three variables:
 * <ul>
 *   <li>char "name": name of the profile</li>
 *   <li>int "isGlobal": this is 0 for profiles loaded from the users home directory and
 *       1 otherwise.</li>
 *   <li>char "fileName": name of the loaded file (without path, so it can be used for
 *       @ref AB_Banking_SaveLocalImExporterProfile)</li>
 * </ul>
 * The remaining content of each group is completely defined by
 * the importer/exporter.
 * </p>
 * <p>
 * You can use @ref GWEN_DB_GetFirstGroup and @ref GWEN_DB_GetNextGroup
 * to browse the profiles.
 * </p>
 * <p>
 * The caller becomes the new owner of the object returned (if any).
 * This makes him/her responsible for freeing it via
 *  @ref GWEN_DB_Group_free.
 * </p>
 * <p>
 * You can use any of the subgroups below the returned one as argument
 * to @ref AB_ImExporter_Import.
 * </p>
 * @param ab pointer to the AB_BANKING object
 * @param name name of the importer whose profiles are to be read
 */
AQBANKING_API
GWEN_DB_NODE *AB_Banking_GetImExporterProfiles(AB_BANKING *ab,
                                               const char *imExporterName);

AQBANKING_API
GWEN_DB_NODE *AB_Banking_GetImExporterProfile(AB_BANKING *ab,
					      const char *imExporterName,
					      const char *profileName);

/**
 * Save the given profile in the local user folder of the given im-/exporter
 * module. After that this profile will appear in the list returned by
 * @ref AB_Banking_GetImExporterProfiles.
 * Existing profiles with the same file name (argument @c fname) will be overwritten.
 * It is best practice to use the name of the profile plus ".conf" as file name
 * (e.g. "testprofile.conf"). The caller has to make sure that the name of the profile
 * is unique among all profiles of the given im-/exporter module, otherwise some
 * profiles can not be loaded.
 *
 * @param ab pointer to the AB_BANKING object
 * @param imexporterName name of the im-/exporter whose profile is to be written
 * @param dbProfile DB group containing the profile
 * @param fname name of the file to write without path (e.g. "testprofile.conf")
 * (if NULL then the path is determined by AqBanking using the given name of the im-/exporter).
 */
AQBANKING_API
int AB_Banking_SaveLocalImExporterProfile(AB_BANKING *ab,
                                          const char *imexporterName,
					  GWEN_DB_NODE *dbProfile,
					  const char *fname);

/**
 * This function should return a dialog (see @ref GWEN_DIALOG) which
 * allows editing of the given profile.
 *
 * @return 0 on success, error code otherwise
 *
 * @param ab pointer to the AB_BANKING object
 * @param imExporterName name of the im-/exporter
 * @param dbProfile configuration data for the exporter. You can get this
 *   using @ref AB_Banking_GetImExporterProfiles.
 * @param pDlg pointer to a dialog pointer (receives the created dialog if any)
 */
AQBANKING_API
int AB_Banking_GetEditImExporterProfileDialog(AB_BANKING *ab,
                                              const char *imExporterName,
                                              GWEN_DB_NODE *dbProfile,
                                              const char *testFileName,
                                              GWEN_DIALOG **pDlg);


/**
 * Reads the given stream and imports all data from it. This imported
 * data is stored within the given context.
 * This is a very basic function, there are convenience functions to make it easier to
 * import from files or buffers.
 *
 * @return 0 on success, error code otherwise
 *
 * @param ab pointer to the AB_BANKING object
 * @param importerName name of the importer module to use
 * @param ctx import context
 * @param sio stream to read from
 * @param dbProfile configuration data for the importer. You can get this
 *   using @ref AB_Banking_GetImExporterProfiles.
 *
 * Example for a dbProfile:
 * @code
 * profile {
 *   char name="test"
 *   char shortDescr="Test"
 *   char version="5.0.4"
 *   char longDescr="Test Profile"
 *   int import="1"
 *   int export="1"
 *   char type="csv"
 *   char groupNames="transaction", "transfer", "debitnote", "line"
 *   char dateFormat="DD.MM.YYYY"
 *   int utc="0"
 *   char subject="transactions"
 *   int usePosNegField="0"
 *   char posNegFieldName="posNeg"
 *   int defaultIsPositive="1"
 *   char valueFormat="float"
 *   params {
 *     quote="0"
 *     title="1"
 *     delimiter=";"
 *     group="transaction"
 *     columns {
 *       1="remoteBankCode"
 *       2="remoteAccountNumber"
 *       3="remoteName[0]"
 *       4="value/value"
 *       5="purpose[0]"
 *       6="purpose[1]"
 *     } # columns
 *   } # params
 *  } # profile
 * @endcode

 */
AQBANKING_API 
int AB_Banking_Import(AB_BANKING *ab,
                      const char *importerName,
                      AB_IMEXPORTER_CONTEXT *ctx,
                      GWEN_SYNCIO *sio,
                      GWEN_DB_NODE *dbProfile);


/**
 * Writes all data to the given stream.
 * This is a very basic function, there are convenience functions to make it easier to
 * export to files or buffers.
 *
 * @return 0 on success, error code otherwise
 *
 * @param ab pointer to the AB_BANKING object
 * @param exporterName name of the exporter module to use
 * @param ctx export context
 * @param sio stream to write to
 * @param dbProfile configuration data for the exporter. You can get this
 *   using @ref AB_Banking_GetImExporterProfiles (see also @ref AB_Banking_Import).
 */
AQBANKING_API 
int AB_Banking_Export(AB_BANKING *ab,
                      const char *exporterName,
                      AB_IMEXPORTER_CONTEXT *ctx,
                      GWEN_SYNCIO *sio,
                      GWEN_DB_NODE *dbProfile);

/*@}*/


/** @name Convenience Functions for Import/Export
 *
 * For import and exports the following objects are generally needed:
 * <ul>
 *   <li>im/exporter module (e.g. "csv", "ofx", "swift")</li>
 *   <li>im/export profile with the settings for the im/exporter module (e.g. "SWIFT-MT940" for
 *       the "swift" importer)</li>
 *   <li>im/export context (needed on import to store imported data, on export to hold the data
 *       to export)</li>
 *   <li>source/destination for the data to import/export (e.g. a CSV-file, OFX file etc)</li>
 * </ul>
 *
 * To make it easier for applications to import/export data this group contains some convenience
 * functions which automatically load the appropriate im/exporter plugin and a selected im/exporter
 * settings profile.
 *
 * The raw im/export API of AqBanking works with GWEN_SYNCIO objects as source/destination for the
 * formatted data. Such a GWEN_SYNCIO object can be a file or a buffer in memory.
 * However, the functions in this group allow you just to specify the file to import from/export to
 * and leave the gory details of setting up a GWEN_SYNCIO to AqBanking.
 *
 * There are functions to:
 * <ul>
 *   <li>import from a file</li>
 *   <li>import from a memory buffer</li>
 *   <li>export to a file</li>
 *   <li>export to a memory buffer</li>
 * </ul>
 */
/*@{*/

/**
 * This function tries to fill missing fields in a given imexporter context.
 * It tries to find the online banking accounts for all account info objects in
 * the context and copies missing information (like IBAN, BIC, owner name etc).
 *
 * @param ab pointer to the AB_BANKING object
 * @param iec pointer to the imexporter context to fill
 * @return 0 if all accounts were found, 1 if there was at least 1 unknown account
 */
AQBANKING_API
int AB_Banking_FillGapsInImExporterContext(AB_BANKING *ab, AB_IMEXPORTER_CONTEXT *iec);


/**
 * Import data using a profile file.
 *
 * The profile to be used is read either from a given profile file (if the argument @b profileFile is given)
 * or from AqBankings internal profile database.
 *
 * @param ab banking API object
 * @param importerName name of the importer module (e.g. "csv", "swift", "ofx" etc)
 * @param ctx import context to receive the imported accounts, transactions etc
 * @param sio IO from which to import
 * @param profileName name of the importer settings profile stored in the file whose name
 *   is given in @b profileFile or the internal profile database
 * @param profileFile name of the file to load the exporter settings profile from. This should
 *  contain at least one profile in a "profile" group. If you want to use profiles installed with
 *  AqBanking you can specify its name via @b profileName and use NULL here
 *
 */
AQBANKING_API 
int AB_Banking_ImportWithProfile(AB_BANKING *ab,
				 const char *importerName,
				 AB_IMEXPORTER_CONTEXT *ctx,
				 GWEN_SYNCIO *sio,
				 const char *profileName,
                                 const char *profileFile);

/**
 * Export data using a profile file.
 *
 * The profile to be used is read either from a given profile file (if the argument @b profileFile is given)
 * or from AqBankings internal profile database.
 *
 * @param ab banking API object
 * @param exporterName name of the exporter module (e.g. "csv", "swift", "ofx" etc)
 * @param ctx context to export
 * @param sio IO to which to export
 * @param profileName name of the exporter settings profile stored in the file whose name
 *   is given in @b profileFile or the internal profile database
 * @param profileFile name of the file to load the exporter settings profile from. This should
 *  contain at least one profile in a "profile" group. If you want to use profiles installed with
 *  AqBanking you can specify its name via @b profileName and use NULL here
 *
 */
AQBANKING_API 
int AB_Banking_ExportWithProfile(AB_BANKING *ab,
				 const char *exporterName,
				 AB_IMEXPORTER_CONTEXT *ctx,
				 GWEN_SYNCIO *sio,
				 const char *profileName,
				 const char *profileFile);


/**
 * Reads the given file and imports all data from it. This imported
 * data is stored within the given context.
 *
 * @return 0 on success, error code otherwise
 *
 * @param ab pointer to the AB_BANKING object
 * @param importerName name of the importer module to use
 * @param ctx import context
 * @param inputFileName path and name of the file to read from
 * @param dbProfile configuration data for the importer. You can get this
 *   using @ref AB_Banking_GetImExporterProfiles.
 */
AQBANKING_API 
int AB_Banking_ImportFromFile(AB_BANKING *ab,
                              const char *importerName,
                              AB_IMEXPORTER_CONTEXT *ctx,
                              const char *inputFileName,
                              GWEN_DB_NODE *dbProfile);

/**
 * Writes data from the given im-/exporter context into a file of the given name.
 * The file will be created if it doesn't exist, otherwise it will be truncated before writing to it.
 *
 * @return 0 on success, error code otherwise
 *
 * @param ab pointer to the AB_BANKING object
 * @param exporterName name of the exporter module to use
 * @param ctx context to export
 * @param outputFileName path and name of the file to write to
 * @param dbProfile configuration data for the exporter. You can get this
 *   using @ref AB_Banking_GetImExporterProfiles.
 */
AQBANKING_API 
int AB_Banking_ExportToFile(AB_BANKING *ab,
                            const char *exporterName,
                            AB_IMEXPORTER_CONTEXT *ctx,
                            const char *outputFileName,
                            GWEN_DB_NODE *dbProfile);


/**
 * Reads the given file and imports all data from it into the given context. Loads profile.
 *
 * The profile to be used is read either from a given profile file (if the argument @b profileFile is given)
 * or from AqBankings internal profile database.
 *
 * @return 0 on success, error code otherwise
 *
 * @param ab pointer to the AB_BANKING object
 * @param importerName name of the importer module to use
 * @param ctx import context
 * @param profileName name of the importer settings profile stored in the file whose name
 *   is given in @b profileFile or the internal profile database
 * @param profileFile name of the file to load the exporter settings profile from. This should
 *  contain at least one profile in a "profile" group. If you want to use profiles installed with
 *  AqBanking you can specify its name via @b profileName and use NULL here
 * @param inputFileName path and name of the file to read from
 */
AQBANKING_API 
int AB_Banking_ImportFromFileWithProfile(AB_BANKING *ab,
                                         const char *importerName,
                                         AB_IMEXPORTER_CONTEXT *ctx,
                                         const char *profileName,
                                         const char *profileFile,
                                         const char *inputFileName);

/**
 * Writes data from the given im-/exporter context into a file of the given name. Loads profile.
 * The file will be created if it doesn't exist, otherwise it will be truncated before writing to it.
 *
 * The profile to be used is read either from a given profile file (if the argument @b profileFile is given)
 * or from AqBankings internal profile database.
 *
 * @return 0 on success, error code otherwise
 *
 * @param ab pointer to the AB_BANKING object
 * @param exporterName name of the exporter module to use
 * @param ctx context to export
 * @param outputFileName path and name of the file to write to
 * @param profileName name of the exporter settings profile stored in the file whose name
 *   is given in @b profileFile or the internal profile database
 * @param profileFile name of the file to load the exporter settings profile from. This should
 *  contain at least one profile in a "profile" group. If you want to use profiles installed with
 *  AqBanking you can specify its name via @b profileName and use NULL here
 */
AQBANKING_API 
int AB_Banking_ExportToFileWithProfile(AB_BANKING *ab,
                                       const char *exporterName,
                                       AB_IMEXPORTER_CONTEXT *ctx,
                                       const char *outputFileName,
                                       const char *profileName,
                                       const char *profileFile);


/**
 * Reads the given memory buffer and reads all data from it. This imported
 * data is stored within the given context.
 *
 * @return 0 on success, error code otherwise
 *
 * @param ab pointer to the AB_BANKING object
 * @param importerName name of the importer module to use
 * @param ctx import context
 * @param dataPtr pointer to the data to import
 * @param dataLen size of the data pointed to by @b dataPtr
 * @param dbProfile configuration data for the importer. You can get this
 *   using @ref AB_Banking_GetImExporterProfiles.
 */
AQBANKING_API 
int AB_Banking_ImportFromBuffer(AB_BANKING *ab,
                                const char *importerName,
                                AB_IMEXPORTER_CONTEXT *ctx,
                                const uint8_t *dataPtr,
                                uint32_t dataLen,
                                GWEN_DB_NODE *dbProfile);

/**
 * Writes data from the given im-/exporter context into a buffer.
 *
 * @return 0 on success, error code otherwise
 *
 * @param ab pointer to the AB_BANKING object
 * @param exporterName name of the exporter module to use
 * @param ctx context to export
 * @param outputBuffer buffer to export data to
 * @param dbProfile configuration data for the exporter. You can get this
 *   using @ref AB_Banking_GetImExporterProfiles.
 */
AQBANKING_API 
int AB_Banking_ExportToBuffer(AB_BANKING *ab,
			      const char *exporterName,
			      AB_IMEXPORTER_CONTEXT *ctx,
                              GWEN_BUFFER *outputBuffer,
			      GWEN_DB_NODE *dbProfile);


/**
 * Reads the given buffer and imports all data from it into the given context. Loads profile.
 *
 * The profile to be used is read either from a given profile file (if the argument @b profileFile is given)
 * or from AqBankings internal profile database.
 *
 * @return 0 on success, error code otherwise
 *
 * @param ab pointer to the AB_BANKING object
 * @param importerName name of the importer module to use
 * @param ctx import context
 * @param profileName name of the importer settings profile stored in the file whose name
 *   is given in @b profileFile or the internal profile database
 * @param profileFile name of the file to load the exporter settings profile from. This should
 *  contain at least one profile in a "profile" group. If you want to use profiles installed with
 *  AqBanking you can specify its name via @b profileName and use NULL here
 * @param dataPtr pointer to the data to import
 * @param dataLen size of the data pointed to by @b dataPtr
 */
AQBANKING_API 
int AB_Banking_ImportFromBufferWithProfile(AB_BANKING *ab,
                                           const char *importerName,
                                           AB_IMEXPORTER_CONTEXT *ctx,
                                           const char *profileName,
                                           const char *profileFile,
                                           const uint8_t *dataPtr,
                                           uint32_t dataLen);

/**
 * Writes data from the given im-/exporter context into a buffer. Loads profile.
 *
 * The profile to be used is read either from a given profile file (if the argument @b profileFile is given)
 * or from AqBankings internal profile database.
 *
 * @return 0 on success, error code otherwise
 *
 * @param ab pointer to the AB_BANKING object
 * @param exporterName name of the exporter module to use
 * @param ctx context to export
 * @param outputBuffer buffer to export data to
 * @param profileName name of the exporter settings profile stored in the file whose name
 *   is given in @b profileFile or the internal profile database
 * @param profileFile name of the file to load the exporter settings profile from. This should
 *  contain at least one profile in a "profile" group. If you want to use profiles installed with
 *  AqBanking you can specify its name via @b profileName and use NULL here
 */
AQBANKING_API 
int AB_Banking_ExportToBufferWithProfile(AB_BANKING *ab,
                                         const char *exporterName,
                                         AB_IMEXPORTER_CONTEXT *ctx,
                                         GWEN_BUFFER *outputBuffer,
                                         const char *profileName,
                                         const char *profileFile);



/*@}*/


/*@}*/ /* addtogroup */

#ifdef __cplusplus
}
#endif

#endif

