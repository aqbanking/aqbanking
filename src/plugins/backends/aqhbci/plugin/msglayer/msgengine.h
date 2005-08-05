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

#ifndef AH_MSGENGINE_H
#define AH_MSGENGINE_H

#ifdef __cplusplus
extern "C" {
#endif
typedef struct AH_MSGENGINE AH_MSGENGINE;
#ifdef __cplusplus
}
#endif

#include <gwenhywfar/msgengine.h>
#include <aqhbci/customer.h>

#ifdef __cplusplus
extern "C" {
#endif

GWEN_MSGENGINE *AH_MsgEngine_new();



void *AH_MsgEngine_GetInheritorData(const GWEN_MSGENGINE *e);
void AH_MsgEngine_SetInheritorData(GWEN_MSGENGINE *e, void *d);
void AH_MsgEngine_SetFreeDataFunction(GWEN_MSGENGINE *e,
                                           GWEN_MSGENGINE_FREEDATA_PTR p);

AH_CUSTOMER *AH_MsgEngine_GetCustomer(const GWEN_MSGENGINE *e);
void AH_MsgEngine_SetCustomer(GWEN_MSGENGINE *e,
                                   AH_CUSTOMER *cu);

#ifdef __cplusplus
}
#endif


#endif /* AH_MSGENGINE_H */

