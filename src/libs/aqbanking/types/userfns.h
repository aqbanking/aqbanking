/***************************************************************************
 begin       : Mon Mar 01 2004
 copyright   : (C) 2018 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 * This file is part of the project "AqBanking".                           *
 * Please see toplevel file COPYING of that project for license details.   *
 ***************************************************************************/



#ifndef AQBANKING_USERFNS_H
#define AQBANKING_USERFNS_H

#include <aqbanking/user.h>


#ifdef __cplusplus
extern "C" {
#endif


/* AQBANKING_API AB_PROVIDER *AB_User_GetProvider(const AB_USER *u); */


AQBANKING_API AB_USER *AB_User_new(AB_BANKING *ab);
AQBANKING_API AB_USER *AB_User_fromDb(AB_BANKING *ab, GWEN_DB_NODE *db);

AQBANKING_API int AB_User_copyDb(GWEN_DB_NODE *dbSrc, GWEN_DB_NODE *dbDst);


#ifdef __cplusplus
} /* __cplusplus */
#endif


#endif /* AQBANKING_USERFNS_H */
