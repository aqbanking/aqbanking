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

#ifndef GB_PROFILELIST_H
#define GB_PROFILELIST_H

#include <gtk/gtk.h>

#include <g2banking/gbanking.h>
#include <aqbanking/banking.h>
#include <aqbanking/job.h>

#include <gwenhywfar/types.h>


/** @defgroup G_AB_G2BANKING_W_PROFLIST Profile List
 * @ingroup G_AB_G2BANKING_WIDGETS
 *
 * This group provides a list widget for im-/exporter profiles.
 */
/*@{*/


/** @name List columns
 *
 */
/*@{*/
/** column which contains the profile name */
#define GB_PROFILELIST_COL_NAME    0
/** column containing the version */
#define GB_PROFILELIST_COL_VERSION 1
/** column containing the short description */
#define GB_PROFILELIST_COL_SHORT   2
/*@}*/



/**
 * Create a new profile list.
 * @param parent parent widget (or 0 if none)
 * @param tree tree widget to use (if NULL then this function will
 * create a new one)
 */
GBANKING_API 
GtkWidget *GB_ProfileList_new(GtkWidget *parent,
                              GtkWidget *tree);

/**
 * Returns a list containing only selected profiles from the given list.
 * The caller is responsible for calling @ref GWEN_DB_Group_free.
 * The jobs in the returned list are only valid as long as the list widget
 * exists without changes.
 * @param w widget created by @ref GB_ProfileList_new
 */
GBANKING_API 
GWEN_DB_NODE *GB_ProfileList_GetSelectedProfiles(GtkWidget *w);

/**
 * This function redraws the profiles in the list.
 * @param w widget created by @ref GB_ProfileList_new
 */
GBANKING_API 
void GB_ProfileList_Update(GtkWidget *w);

/**
 * Clear the list of profiles.
 * @param w widget created by @ref GB_ProfileList_new
 */
GBANKING_API 
void GB_ProfileList_Clear(GtkWidget *w);

/**
 * Add a profile to the list.
 * @param w widget created by @ref GB_ProfileList_new
 * @param db GWEN_DB_NODE containing a profile. This function makes a deep
 * copy.
 */
GBANKING_API 
void GB_ProfileList_AddProfile(GtkWidget *w, GWEN_DB_NODE *db);

/**
 * Add multiple profiles to the list.
 * @param w widget created by @ref GB_ProfileList_new
 * @param db GWEN_DB_NODE containing the profiles in subgroups.
 * This function makes a deep copy.
 */
GBANKING_API 
void GB_ProfileList_AddProfiles(GtkWidget *w, GWEN_DB_NODE *db);
/*@}*/ /* defgroup */


#endif



