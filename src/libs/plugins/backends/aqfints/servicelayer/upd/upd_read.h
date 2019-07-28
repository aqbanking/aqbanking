/***************************************************************************
 begin       : Sun Jul 28 2019
 copyright   : (C) 2019 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 * This file is part of the project "AqBanking".                           *
 * Please see toplevel file COPYING of that project for license details.   *
 ***************************************************************************/

#ifndef UPD_READ_H
#define UPD_READ_H


#include "servicelayer/upd/accountdata.h"
#include "servicelayer/upd/userdata.h"

#include <gwenhywfar/db.h>


/**
 * Read a AQFINTS_USERDATA from a DB created by @ref AQFINTS_Parser_ReadSegmentListToDb().
 *
 * @return AQFINTS_USERDATA object (asserts on error)
 * @param db db node with a "UserData" group
 */
AQFINTS_USERDATA *AQFINTS_Upd_ReadUserData(GWEN_DB_NODE *db);


/**
 * Read a AQFINTS_ACCOUNTDATA from a DB created by @ref AQFINTS_Parser_ReadSegmentListToDb().
 *
 * @return AQFINTS_ACCOUNTDATA object (asserts on error)
 * @param db db node with a "AccountData" group
 */
AQFINTS_ACCOUNTDATA *AQFINTS_Upd_ReadAccountData(GWEN_DB_NODE *db);



#endif

