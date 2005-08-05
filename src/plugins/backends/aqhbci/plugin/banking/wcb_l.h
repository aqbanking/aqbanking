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

#ifndef AH_WCB_L_H
#define AH_WCB_L_H

#include <aqhbci/hbci.h>
#include <gwenhywfar/waitcallback.h>


GWEN_WAITCALLBACK *AH_WaitCallback_new(AH_HBCI *hbci, const char *id);











#endif /* AH_WCB_L_H */




