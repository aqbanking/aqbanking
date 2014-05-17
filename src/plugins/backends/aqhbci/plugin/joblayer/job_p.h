/***************************************************************************
    begin       : Mon Mar 01 2004
    copyright   : (C) 2004-2011 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#ifndef AH_JOB_P_H
#define AH_JOB_P_H

#include "job_l.h"
#include <gwenhywfar/stringlist.h>
#include <gwenhywfar/msgengine.h>



struct AH_JOB {
  GWEN_LIST_ELEMENT(AH_JOB);
  GWEN_INHERIT_ELEMENT(AH_JOB);

  char *name;
  char *code;
  char *accountId;
  char *description;
  char *responseName;

  int segmentVersion;

  int challengeClass;

  char *expectedSigner;
  char *expectedCrypter;

  char *usedTan;

  AB_USER *user;

  unsigned int msgNum;
  char *dialogId;
  uint32_t firstSegment;
  uint32_t lastSegment;

  GWEN_STRINGLIST *signers;
  GWEN_STRINGLIST *sepaDescriptors;

  GWEN_XMLNODE *xmlNode;
  GWEN_XMLNODE *msgNode;
  GWEN_DB_NODE *jobParams;
  GWEN_DB_NODE *jobArguments;
  GWEN_DB_NODE *jobResponses;
  GWEN_DB_NODE *sepaProfile;

  AH_JOB_STATUS status;
  uint32_t flags;
  int minSigs;
  int secProfile;
  int secClass;
  int jobsPerMsg;

  GWEN_MSGENGINE *msgEngine;

  uint32_t usage;

  AH_JOB_PROCESS_FN processFn;
  AH_JOB_COMMIT_FN commitFn;
  AH_JOB_PREPARE_FN prepareFn;

  AH_JOB_EXCHANGE_FN exchangeFn;

  AH_JOB_NEXTMSG_FN nextMsgFn;

  AH_JOB_ADDCHALLENGEPARAMS_FN addChallengeParamsFn;

  AH_RESULT_LIST *segResults;
  AH_RESULT_LIST *msgResults;

  uint32_t id;

  AB_MESSAGE_LIST *messages;

  GWEN_STRINGLIST *log;

  GWEN_STRINGLIST *challengeParams;
  AB_VALUE *challengeValue;

  int maxTransfers;
  int transferCount;
  AB_TRANSACTION_LIST *transferList;
};


static void AH_Job_SampleResults(AH_JOB *j);
static AH_JOB *AH_Job__freeAll_cb(AH_JOB *j, void *userData);

static int AH_Job__CommitSystemData(AH_JOB *j, int doLock);


#endif /* AH_JOB_P_H */




