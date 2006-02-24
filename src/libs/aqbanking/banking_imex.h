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


#ifndef AQBANKING_BANKING_IMEX_H
#define AQBANKING_BANKING_IMEX_H

#include <aqbanking/imexporter.h>


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
 * Loads an importer/exporter backend with the given name. You can use
 * @ref AB_Banking_GetImExporterDescrs to retrieve a list of available
 * backends.
 * AqBanking remains the owner of the object returned (if any), so you
 * <b>must not</b> free it.
 */
AQBANKING_API 
AB_IMEXPORTER *AB_Banking_GetImExporter(AB_BANKING *ab, const char *name);

/**
 * <p>
 * Loads all available profiles for the given importer/exporter.
 * This includes global profiles as well as local ones.
 * </p>
 * <p>
 * Local profiles overwrite global ones, allowing the user to customize the
 * profiles.
 * </p>
 * <p>
 * The GWEN_DB returned contains one group for every loaded profile. Every
 * group has the name of the profile it contains. Every group contains at
 * least a variable called <i>name</i> which contains the name of the
 * profile, too. The remaining content of each group is completely defined by
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
                                               const char *name);
/*@}*/


/*@}*/ /* addtogroup */

#ifdef __cplusplus
}
#endif

#endif

