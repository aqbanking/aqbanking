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

/**
 * This update adds the variable "tanMethods" with an initial value of
 * "singleStep"
 */
static int AH_HBCI_UpdateUser_2_1_1_1(AH_HBCI *hbci, GWEN_DB_NODE *db);

/**
 * This update creates the crypt token settings from the medium id
 * and sets the rdhType if it not already is.
 */
static int AH_HBCI_UpdateUser_2_9_3_2(AH_HBCI *hbci, GWEN_DB_NODE *db);

/**
 * This update creates tokenContextId from contextId for media.
 */
static int AH_HBCI_UpdateUser_2_9_3_3(AH_HBCI *hbci, GWEN_DB_NODE *db);

/*@}*/


/** @name Account DB Update Functions
 *
 * Functions in this group called from @ref AB_Account_Extend.
 */
/*@{*/
/**
 * This update sets the account flags to the default value if they are "0".
 * This now makes single transfers the default.
 */
static int AH_HBCI_UpdateAccount_1_9_7_9(AH_HBCI *hbci, GWEN_DB_NODE *db);
/*@}*/


/** @name Newer Update Functions
 *
 * Functions of this group are called from AB_Provider_Update after any of
 * the init update functions have been called.
 */
/*@{*/


/**
 * This function just aborts with a message.
 */
static int AH_HBCI_Update2_1_8_1_3(AH_HBCI *hbci, GWEN_DB_NODE *db);


/**
 * This function removes the "media" section of the configuration file
 * since AqHBCI no longer uses media.
 */
static int AH_HBCI_Update2_2_9_3_3(AH_HBCI *hbci, GWEN_DB_NODE *db);

/*@}*/

#endif

