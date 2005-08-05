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


#ifndef AH_CONNECTION_HBCI_H
#define AH_CONNECTION_HBCI_H


#define AH_CONNECTION_HBCI_MARK_HBCI  1
#define AH_CONNECTION_HBCI_MARK_HTTPS 2

#include <gwenhywfar/netconnection.h>


#ifdef __cplusplus
extern "C" {
#endif

GWEN_NETCONNECTION *AH_ConnectionHBCI_new(GWEN_NETTRANSPORT *tr,
                                          int take,
                                          GWEN_TYPE_UINT32 libId);

void AH_ConnectionHBCI_SetDownAfterSend(GWEN_NETCONNECTION *conn, int i);


#ifdef __cplusplus
}
#endif


#endif /* AH_CONNECTION_HBCI_H */


