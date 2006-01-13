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



#ifndef AQBANKING_USERFNS_L_H
#define AQBANKING_USERFNS_L_H

#include "userfns.h"


AB_USER *AB_User_new(AB_BANKING *ab);
int AB_User__dbToDb(GWEN_DB_NODE *n, GWEN_DB_NODE *where);



#endif /* AQBANKING_USERFNS_L_H */
