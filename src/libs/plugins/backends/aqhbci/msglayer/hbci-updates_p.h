/***************************************************************************
    begin       : Mon Mar 01 2004
    copyright   : (C) 2019 by Martin Preuss
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
 * This update transforms selectedTanMethod
 */
static int AH_HBCI_UpdateUser_5_0_3_1(AH_HBCI *hbci, GWEN_DB_NODE *db);

/*@}*/



#endif

