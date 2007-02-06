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


typedef struct AO_USER AO_USER;
struct AO_USER {
  GWEN_TYPE_UINT32 flags;
};

static void GWENHYWFAR_CB AO_User_FreeData(void *bp, void *p);



#endif
