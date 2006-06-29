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

#ifndef AH_USER_P_H
#define AH_USER_P_H

#include "user_l.h"


typedef struct AH_USER AH_USER;
struct AH_USER {
  AH_HBCI *hbci;
  GWEN_MSGENGINE *msgEngine;

  AH_MEDIUM *medium;

  GWEN_URL *serverUrl;
  AH_BPD *bpd;

  GWEN_TYPE_UINT32 flags;
  GWEN_TYPE_UINT32 tanMethods;
  GWEN_TYPE_UINT32 selectedTanMethod;
};

static void AH_User_freeData(void *bp, void *p);



#endif /* AH_USER_P_H */


