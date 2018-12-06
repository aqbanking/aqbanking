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

/**
 * This update transforms tanMethods to tanMethodList
 */
static int AH_HBCI_UpdateUser_3_1_1_2(AH_HBCI *hbci, GWEN_DB_NODE *db);

/**
 * This update transforms selectedTanMethod
 */
static int AH_HBCI_UpdateUser_5_0_3_1(AH_HBCI *hbci, GWEN_DB_NODE *db);

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


#endif

