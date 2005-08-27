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


#ifndef AH_JOB_P_H
#define AH_JOB_P_H

#include "job_l.h"
#include <gwenhywfar/stringlist.h>



struct AH_JOB {
  GWEN_LIST_ELEMENT(AH_JOB);
  GWEN_INHERIT_ELEMENT(AH_JOB);

  char *name;
  char *accountId;
  char *description;

  char *expectedSigner;
  char *expectedCrypter;

  char *usedTan;

  AH_CUSTOMER *customer;

  unsigned int msgNum;
  char *dialogId;
  GWEN_TYPE_UINT32 firstSegment;
  GWEN_TYPE_UINT32 lastSegment;

  GWEN_STRINGLIST *signers;

  GWEN_XMLNODE *xmlNode;
  GWEN_XMLNODE *msgNode;
  GWEN_DB_NODE *jobParams;
  GWEN_DB_NODE *jobArguments;
  GWEN_DB_NODE *jobResponses;

  AH_JOB_STATUS status;
  GWEN_TYPE_UINT32 flags;
  int minSigs;
  int jobsPerMsg;

  GWEN_MSGENGINE *msgEngine;

  GWEN_TYPE_UINT32 usage;

  AH_JOB_PROCESS_FN processFn;
  AH_JOB_COMMIT_FN commitFn;

  AH_JOB_EXCHANGE_FN exchangeFn;

  AH_JOB_NEXTMSG_FN nextMsgFn;

  AH_RESULT_LIST *segResults;
  AH_RESULT_LIST *msgResults;

  GWEN_TYPE_UINT32 id;

  GWEN_STRINGLIST *log;
};


void AH_Job_SampleResults(AH_JOB *j);
AH_JOB *AH_Job__freeAll_cb(AH_JOB *j, void *userData);


#endif /* AH_JOB_P_H */




