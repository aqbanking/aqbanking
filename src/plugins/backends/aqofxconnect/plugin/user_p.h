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
  uint32_t flags;

  char *brokerId;
  char *org;
  char *fid;
  AO_USER_SERVERTYPE serverType;
  char *serverAddr;
  int serverPort;
  char *appId;
  char *appVer;
  char *headerVer;
};

static void GWENHYWFAR_CB AO_User_FreeData(void *bp, void *p);



#endif
