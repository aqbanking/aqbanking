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



#ifndef AQBANKING_USERFNS_H
#define AQBANKING_USERFNS_H

#include <aqbanking/user.h>


#ifdef __cplusplus
extern "C" {
#endif


AQBANKING_API 
AB_USER *AB_User_fromDb(AB_BANKING *ab, GWEN_DB_NODE *db);

AQBANKING_API
AB_PROVIDER *AB_User_GetProvider(const AB_USER *u);

AQBANKING_API 
GWEN_DB_NODE *AB_User_GetProviderData(const AB_USER *u);

AQBANKING_API 
GWEN_DB_NODE *AB_User_GetAppData(const AB_USER *u);


#ifdef __cplusplus
} /* __cplusplus */
#endif


#endif /* AQBANKING_USERFNS_H */
