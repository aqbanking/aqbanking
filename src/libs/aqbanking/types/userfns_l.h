/***************************************************************************
 begin       : Mon Mar 01 2004
 copyright   : (C) 2004 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 * This file is part of the project "AqBanking".                           *
 * Please see toplevel file COPYING of that project for license details.   *
 ***************************************************************************/



#ifndef AQBANKING_USERFNS_L_H
#define AQBANKING_USERFNS_L_H

#include "userfns.h"


AB_USER *AB_User_new(AB_BANKING *ab);
AB_USER *AB_User_fromDb(AB_BANKING *ab, GWEN_DB_NODE *db);



#endif /* AQBANKING_USERFNS_L_H */
