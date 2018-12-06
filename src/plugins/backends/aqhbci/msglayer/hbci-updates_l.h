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


#ifndef AH_HBCI_UPDATES_L_H
#define AH_HBCI_UPDATES_L_H

#include "hbci_l.h"
#include <gwenhywfar/types.h>

/**
 * This function is called from AH_User_Extend to extend the given user data
 * DB to the latest version.
 */
int AH_HBCI_UpdateDbUser(AH_HBCI *hbci, GWEN_DB_NODE *db);

/**
 * This function is called from AH_User_Extend to extend the given account
 * data DB to the latest version.
 */
int AH_HBCI_UpdateDbAccount(AH_HBCI *hbci, GWEN_DB_NODE *db);


#endif

