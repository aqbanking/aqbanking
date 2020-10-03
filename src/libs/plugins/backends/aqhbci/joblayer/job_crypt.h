/***************************************************************************
    begin       : Mon Mar 01 2004
    copyright   : (C) 2018 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifndef AH_JOB_CRYPT_H
#define AH_JOB_CRYPT_H

#include "aqhbci/joblayer/job_l.h"

#include <gwenhywfar/db.h>



int AH_Job_CheckEncryption(AH_JOB *j, GWEN_DB_NODE *dbRsp);
int AH_Job_CheckSignature(AH_JOB *j, GWEN_DB_NODE *dbRsp);


#endif




