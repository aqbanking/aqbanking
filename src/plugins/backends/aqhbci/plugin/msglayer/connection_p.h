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


#ifndef AH_CONNECTION_P_H
#define AH_CONNECTION_P_H

#include <aqhbci/connection.h>


typedef struct AH_CONNECTION AH_CONNECTION;
struct AH_CONNECTION {
  AH_HBCI *hbci;
  int connectionDown;
};


static void AH_Connection_FreeData(void *bp, void *p);

static void AH_Connection_Up(GWEN_NETCONNECTION *conn);
static void AH_Connection_Down(GWEN_NETCONNECTION *conn);


#endif /* AH_CONNECTION_P_H */


