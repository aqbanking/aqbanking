/***************************************************************************
 $RCSfile: user_l.h,v $
                             -------------------
    cvs         : $Id: user_l.h,v 1.2 2006/01/13 13:59:59 cstim Exp $
    begin       : Mon Mar 01 2004
    copyright   : (C) 2004 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifndef EBC_CLIENT_USER_L_H
#define EBC_CLIENT_USER_L_H


#include <aqebics/user.h>
#include <aqbanking/provider_be.h>


void EBC_User_Extend(AB_USER *u, AB_PROVIDER *pro,
		     AB_PROVIDER_EXTEND_MODE em,
		     GWEN_DB_NODE *db);


#endif /* EBC_CLIENT_USER_L_H */






