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

#include <aqhbci/aqhbci.h>
#include <aqbanking/user.h>
#include <gwenhywfar/msgengine.h>

#ifdef __cplusplus
extern "C" {
#endif

AQHBCI_API
GWEN_MSGENGINE *AH_MsgEngine_new();


void AH_MsgEngine_SetUser(GWEN_MSGENGINE *e, AB_USER *u);

#ifdef __cplusplus
}
#endif


#endif /* AH_MSGENGINE_H */

