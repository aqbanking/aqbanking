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


#ifndef AH_HBCI_UPDATES_P_H
#define AH_HBCI_UPDATES_P_H

#include "hbci-updates_l.h"


/** @name Init Update Functions
 *
 * Functions of this group are called from AB_Provider_Init().
 */
/*@{*/

/**
 * This update only shows a warning about AqHBCI no longer writing the
 * pin into logfiles.
 */
static int AH_HBCI_Update_1_0_3_9(AH_HBCI *hbci, GWEN_DB_NODE *db);

/**
 * This update adjusts the medium storage to the new structure used in
 * AqHBCI: The list of known media is now stored globally within AqHBCI,
 * and each user now only links to the global media.
 */
static int AH_HBCI_Update_1_2_0_3(AH_HBCI *hbci, GWEN_DB_NODE *db);

/**
 * This update clears all context lists in all media so that they will
 * be read again.
 */
static int AH_HBCI_Update_1_4_1_2(AH_HBCI *hbci, GWEN_DB_NODE *db);

/**
 * This init update assigns a unique id to every medium.
 */
static int AH_HBCI_Update_1_8_1_3(AH_HBCI *hbci, GWEN_DB_NODE *db);

/**
 * This update sets the bpdVersion and updVersion of all customers
 * to zero. Therefore upon next connect with the bank server the UPD and BPD
 * will be updated automatically.
 */
static int AH_HBCI_Update_Any(AH_HBCI *hbci, GWEN_DB_NODE *db);

/*@}*/


/** @name User DB Update Functions
 *
 * Functions in this group called from @ref AB_User_Extend.
 */
/*@{*/
/**
 * This update converts AH_BPDADDR groups to GWEN_URL strings.
 */
static int AH_HBCI_UpdateUser_1_9_7_7(AH_HBCI *hbci, GWEN_DB_NODE *db);
/*@}*/



/** @name Newer Update Functions
 *
 * Functions of this group are called from AB_Provider_Update after any of
 * the init update functions have been called.
 */
/*@{*/


/**
 * This function creates users and accounts from an existing AqBanking/AqHBCI
 * setup.
 */
static int AH_HBCI_Update2_1_8_1_3(AH_HBCI *hbci, GWEN_DB_NODE *db);


/*@}*/

#endif

