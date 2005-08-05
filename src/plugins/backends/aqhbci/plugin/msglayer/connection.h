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


#ifndef AH_CONNECTION_H
#define AH_CONNECTION_H


#define AH_CONNECTION_MARK_HBCI  1
#define AH_CONNECTION_MARK_HTTPS 2

#include <gwenhywfar/netconnection.h>
#include <aqhbci/hbci.h>

#ifdef __cplusplus
extern "C" {
#endif

void AH_Connection_Extend(AH_HBCI *hbci, GWEN_NETCONNECTION *conn);

int AH_Connection_IsDown(const GWEN_NETCONNECTION *conn);
void AH_Connection_SetIsDown(GWEN_NETCONNECTION *conn, int i);

#ifdef __cplusplus
}
#endif


#endif /* AH_CONNECTION_H */


