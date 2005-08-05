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

#ifndef AH_OUTBOX_L_H
#define AH_OUTBOX_L_H


#include <aqhbci/outbox.h>

#ifdef __cplusplus
extern "C" {
#endif


AH_JOB_LIST *AH_Outbox_GetFinishedJobs(AH_OUTBOX *ob);

#ifdef __cplusplus
}
#endif


#endif /* AH_OUTBOX_L_H */

