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

#ifndef AO_USER_P_H
#define AO_USER_P_H

#include <aqofxconnect/user.h>


struct AO_USER {
  GWEN_LIST_ELEMENT(AO_USER)
  AO_BANK *bank;
  char *userId;
  char *userName;
};



#endif
