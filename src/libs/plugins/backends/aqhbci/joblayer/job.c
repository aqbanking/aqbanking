/***************************************************************************
    begin       : Mon Mar 01 2004
    copyright   : (C) 2018 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#ifdef HAVE_CONFIG_H
# include <config.h>
#endif


#include "job_p.h"

#include "aqhbci/aqhbci_l.h"
#include "aqhbci/msglayer/hbci_l.h"
#include "aqhbci/banking/user_l.h"
#include "aqhbci/banking/account_l.h"
#include "aqhbci/banking/provider_l.h"
#include "aqhbci/banking/provider.h"

#include <aqbanking/backendsupport/account.h>

#include <gwenhywfar/debug.h>
#include <gwenhywfar/misc.h>
#include <gwenhywfar/gui.h>
#include <gwenhywfar/text.h>
#include <gwenhywfar/stringlist.h>
#include <gwenhywfar/cryptkeyrsa.h>

#include <assert.h>



GWEN_LIST_FUNCTIONS(AH_JOB, AH_Job);
GWEN_LIST2_FUNCTIONS(AH_JOB, AH_Job);
GWEN_INHERIT_FUNCTIONS(AH_JOB);





static void _flagsToBuffer(uint32_t flags, GWEN_BUFFER *dbuf);




void AH_Job_free(AH_JOB *j)
{
  if (j) {
    assert(j->usage);
    if (--(j->usage)==0) {
      GWEN_StringList_free(j->challengeParams);
      GWEN_StringList_free(j->log);
      GWEN_StringList_free(j->signers);
      GWEN_StringList_free(j->sepaDescriptors);
      free(j->responseName);
      free(j->code);
      free(j->name);
      free(j->dialogId);
      free(j->expectedSigner);
      free(j->expectedCrypter);
      free(j->usedTan);
      GWEN_MsgEngine_free(j->msgEngine);
      GWEN_DB_Group_free(j->jobParams);
      GWEN_DB_Group_free(j->jobArguments);
      GWEN_DB_Group_free(j->jobResponses);
      GWEN_DB_Group_free(j->sepaProfile);
      AH_Result_List_free(j->msgResults);
      AH_Result_List_free(j->segResults);
      AB_Message_List_free(j->messages);

      AB_Transaction_List_free(j->transferList);
      AB_Transaction_List2_free(j->commandList);

      GWEN_LIST_FINI(AH_JOB, j);
      GWEN_INHERIT_FINI(AH_JOB, j);
      GWEN_FREE_OBJECT(j);
    }
  }
}



int AH_Job_SampleBpdVersions(const char *name,
                             AB_USER *u,
                             GWEN_DB_NODE *dbResult)
{
  GWEN_XMLNODE *node;
  const char *paramName;
  GWEN_DB_NODE *bpdgrp;
  const AH_BPD *bpd;
  GWEN_MSGENGINE *e;

  assert(name);
  assert(u);

  DBG_INFO(AQHBCI_LOGDOMAIN, "Sampling BPD versions for job \"%s\"", name?name:"<noname>");

  /* get job descriptions */
  e=AH_User_GetMsgEngine(u);
  assert(e);

  bpd=AH_User_GetBpd(u);

  if (AH_User_GetHbciVersion(u)==0)
    GWEN_MsgEngine_SetProtocolVersion(e, 210);
  else
    GWEN_MsgEngine_SetProtocolVersion(e, AH_User_GetHbciVersion(u));

  GWEN_MsgEngine_SetMode(e, AH_CryptMode_toString(AH_User_GetCryptMode(u)));

  /* first select any version, we simply need to know the BPD job name */
  node=GWEN_MsgEngine_FindNodeByProperty(e,
                                         "JOB",
                                         "id",
                                         0,
                                         name);
  if (!node) {
    DBG_INFO(AQHBCI_LOGDOMAIN,
             "Job \"%s\" not supported by local XML files", name);
    return GWEN_ERROR_NOT_FOUND;
  }

  /* get some properties */
  paramName=GWEN_XMLNode_GetProperty(node, "params", "");

  if (bpd) {
    bpdgrp=AH_Bpd_GetBpdJobs(bpd, AH_User_GetHbciVersion(u));
    assert(bpdgrp);
  }
  else
    bpdgrp=NULL;

  if (paramName && *paramName) {
    GWEN_DB_NODE *jobBPD;

    DBG_INFO(AQHBCI_LOGDOMAIN, "Job \"%s\" needs BPD job \"%s\"",
             name, paramName);

    if (!bpd) {
      DBG_ERROR(AQHBCI_LOGDOMAIN, "No BPD");
      return GWEN_ERROR_BAD_DATA;
    }

    /* get BPD job */
    jobBPD=GWEN_DB_GetGroup(bpdgrp,
                            GWEN_PATH_FLAGS_NAMEMUSTEXIST,
                            paramName);
    if (jobBPD) {
      /* children are one group per version */
      jobBPD=GWEN_DB_GetFirstGroup(jobBPD);
    }

    /* check for jobs for which we have a BPD */
    while (jobBPD) {
      int version;

      /* get version from BPD */
      version=atoi(GWEN_DB_GroupName(jobBPD));
      /* now get the correct version of the JOB */
      DBG_DEBUG(AQHBCI_LOGDOMAIN, "Checking Job %s (%d)", name, version);
      node=GWEN_MsgEngine_FindNodeByProperty(e,
                                             "JOB",
                                             "id",
                                             version,
                                             name);
      if (node) {
        GWEN_DB_NODE *cpy;

        cpy=GWEN_DB_Group_dup(jobBPD);
        GWEN_DB_AddGroup(dbResult, cpy);
      }
      jobBPD=GWEN_DB_GetNextGroup(jobBPD);
    } /* while */
  } /* if paramName */
  else {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "Job has no BPDs");
    return 0;
  }

  return 0;
}



int AH_Job_GetMaxVersionUpUntil(const char *name, AB_USER *u, int maxVersion)
{
  GWEN_DB_NODE *db;
  int rv;

  db=GWEN_DB_Group_new("bpd");
  rv=AH_Job_SampleBpdVersions(name, u, db);
  if (rv<0) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
    GWEN_DB_Group_free(db);
    return rv;
  }
  else {
    GWEN_DB_NODE *dbT;
    int m=-1;

    /* determine maximum version */
    dbT=GWEN_DB_GetFirstGroup(db);
    while (dbT) {
      int v;

      v=atoi(GWEN_DB_GroupName(dbT));
      if (v>0 && v>m && v<=maxVersion)
        m=v;
      dbT=GWEN_DB_GetNextGroup(dbT);
    }
    GWEN_DB_Group_free(db);
    DBG_DEBUG(AQHBCI_LOGDOMAIN, "Max version of [%s] up until %d: %d",
              name, maxVersion, m);
    return m;
  }
}



AB_MESSAGE_LIST *AH_Job_GetMessages(const AH_JOB *j)
{
  assert(j);
  assert(j->usage);
  return j->messages;
}



int AH_Job_GetChallengeClass(const AH_JOB *j)
{
  assert(j);
  assert(j->usage);
  return j->challengeClass;
}



int AH_Job_GetSegmentVersion(const AH_JOB *j)
{
  assert(j);
  assert(j->usage);
  return j->segmentVersion;
}



void AH_Job_SetChallengeClass(AH_JOB *j, int i)
{
  assert(j);
  assert(j->usage);
  j->challengeClass=i;
}



void AH_Job_Attach(AH_JOB *j)
{
  assert(j);
  assert(j->usage);
  j->usage++;
}



int AH_Job_PrepareNextMessage(AH_JOB *j)
{
  assert(j);
  assert(j->usage);

  if (j->nextMsgFn) {
    int rv;

    rv=j->nextMsgFn(j);
    if (rv==0) {
      /* callback flagged that no message follows */
      DBG_INFO(AQHBCI_LOGDOMAIN, "Job \"%s\" says: No more messages", j->name);
      AH_Job_SubFlags(j, AH_JOB_FLAGS_HASMOREMSGS);
      return 0;
    }
    else if (rv!=1) {
      DBG_INFO(AQHBCI_LOGDOMAIN, "Job \"%s\" says: Error", j->name);
      AH_Job_SubFlags(j, AH_JOB_FLAGS_HASMOREMSGS);
      return rv;
    }
  }

  if (j->status==AH_JobStatusUnknown ||
      j->status==AH_JobStatusError) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "At least one message had errors, aborting job \"%s\"", j->name);
    AH_Job_SubFlags(j, AH_JOB_FLAGS_HASMOREMSGS);
    return 0;
  }

  if (j->status==AH_JobStatusToDo) {
    DBG_NOTICE(AQHBCI_LOGDOMAIN,
               "Hmm, job \"%s\" has never been sent, so we do nothing here", j->name);
    AH_Job_SubFlags(j, AH_JOB_FLAGS_HASMOREMSGS);
    return 0;
  }

  if (AH_Job_GetFlags(j) & AH_JOB_FLAGS_HASATTACHPOINT) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "Job \"%s\" has an attachpoint, so yes, we need more messages", j->name);
    AH_Job_AddFlags(j, AH_JOB_FLAGS_HASMOREMSGS);
    AH_Job_Log(j, GWEN_LoggerLevel_Debug, "Job has an attachpoint");
    return 1;
  }
  else {
    DBG_INFO(AQHBCI_LOGDOMAIN, "Job \"%s\" has no attachpoint", j->name);
  }

  if (!(AH_Job_GetFlags(j) & AH_JOB_FLAGS_MULTIMSG)) {
    DBG_NOTICE(AQHBCI_LOGDOMAIN, "Not a multi-message job \"%s\", so we don't need more messages", j->name);
    AH_Job_SubFlags(j, AH_JOB_FLAGS_HASMOREMSGS);
    return 0;
  }

  DBG_INFO(AQHBCI_LOGDOMAIN, "Multi-message job \"%s\", looking for next message", j->name);
  assert(j->msgNode);
  j->msgNode=GWEN_XMLNode_FindNextTag(j->msgNode, "MESSAGE", 0, 0);
  if (j->msgNode) {
    /* there is another message, so set flags accordingly */
    DBG_NOTICE(AQHBCI_LOGDOMAIN, "Multi-message job \"%s\", still more messages", j->name);
    AH_Job_Log(j, GWEN_LoggerLevel_Debug,
               "Job has more messages");

    /* sample some flags for the next message */
    if (atoi(GWEN_XMLNode_GetProperty(j->msgNode, "sign", "1"))!=0) {
      if (j->minSigs==0)
        j->minSigs=1;
      AH_Job_AddFlags(j, AH_JOB_FLAGS_NEEDSIGN | AH_JOB_FLAGS_SIGN);
    }
    else {
      AH_Job_SubFlags(j, AH_JOB_FLAGS_NEEDSIGN | AH_JOB_FLAGS_SIGN);
    }
    if (atoi(GWEN_XMLNode_GetProperty(j->msgNode, "crypt", "1"))!=0)
      AH_Job_AddFlags(j, AH_JOB_FLAGS_NEEDCRYPT| AH_JOB_FLAGS_CRYPT);
    else
      AH_Job_SubFlags(j, AH_JOB_FLAGS_NEEDCRYPT| AH_JOB_FLAGS_CRYPT);

    if (atoi(GWEN_XMLNode_GetProperty(j->msgNode, "nosysid", "0"))!=0)
      AH_Job_AddFlags(j, AH_JOB_FLAGS_NOSYSID);
    else
      AH_Job_SubFlags(j, AH_JOB_FLAGS_NOSYSID);

    if (atoi(GWEN_XMLNode_GetProperty(j->msgNode, "signseqone", "0"))!=0)
      AH_Job_AddFlags(j, AH_JOB_FLAGS_SIGNSEQONE);
    else
      AH_Job_SubFlags(j, AH_JOB_FLAGS_SIGNSEQONE);

    if (atoi(GWEN_XMLNode_GetProperty(j->msgNode, "noitan", "0"))!=0) {
      AH_Job_AddFlags(j, AH_JOB_FLAGS_NOITAN);
    }
    else
      AH_Job_SubFlags(j, AH_JOB_FLAGS_NOITAN);

    if (atoi(GWEN_XMLNode_GetProperty(j->msgNode, "ignerrors", "0"))!=0)
      AH_Job_AddFlags(j, AH_JOB_FLAGS_IGNORE_ERROR);
    else
      AH_Job_SubFlags(j, AH_JOB_FLAGS_IGNORE_ERROR);

    AH_Job_AddFlags(j, AH_JOB_FLAGS_HASMOREMSGS);
    return 1;
  }
  else {
    DBG_INFO(AQHBCI_LOGDOMAIN, "Multi-message job \"%s\" is finished", j->name);
    AH_Job_Log(j, GWEN_LoggerLevel_Debug,
               "Job has no more messages");
    AH_Job_SubFlags(j, AH_JOB_FLAGS_HASMOREMSGS);
    return 0;
  }
}



uint32_t AH_Job_GetId(const AH_JOB *j)
{
  assert(j);
  assert(j->usage);
  return j->id;
}



void AH_Job_SetId(AH_JOB *j, uint32_t i)
{
  assert(j);
  assert(j->usage);
  j->id=i;
}



const char *AH_Job_GetName(const AH_JOB *j)
{
  assert(j);
  assert(j->usage);
  return j->name;
}



const char *AH_Job_GetCode(const AH_JOB *j)
{
  assert(j);
  assert(j->usage);
  return j->code;
}



void AH_Job_SetCode(AH_JOB *j, const char *s)
{
  assert(j);
  assert(j->usage);
  if (j->code)
    free(j->code);
  if (s)
    j->code=strdup(s);
  else
    j->code=NULL;
}



const char *AH_Job_GetResponseName(const AH_JOB *j)
{
  assert(j);
  assert(j->usage);
  return j->responseName;
}



void AH_Job_SetResponseName(AH_JOB *j, const char *s)
{
  assert(j);
  assert(j->usage);
  if (j->responseName)
    free(j->responseName);
  if (s)
    j->responseName=strdup(s);
  else
    j->responseName=NULL;
}



int AH_Job_GetMinSignatures(const AH_JOB *j)
{
  assert(j);
  assert(j->usage);
  return j->minSigs;
}



int AH_Job_GetSecurityProfile(const AH_JOB *j)
{
  assert(j);
  assert(j->usage);
  return j->secProfile;
}



int AH_Job_GetSecurityClass(const AH_JOB *j)
{
  assert(j);
  assert(j->usage);
  return j->secClass;
}



int AH_Job_GetJobsPerMsg(const AH_JOB *j)
{
  assert(j);
  assert(j->usage);
  return j->jobsPerMsg;
}



uint32_t AH_Job_GetFlags(const AH_JOB *j)
{
  assert(j);
  assert(j->usage);
  return j->flags;
}



void AH_Job_SetFlags(AH_JOB *j, uint32_t f)
{
  assert(j);
  assert(j->usage);

  if (j->flags!=f) {
    GWEN_BUFFER *bufBefore;
    GWEN_BUFFER *bufAfter;

    bufBefore=GWEN_Buffer_new(0, 128, 0, 1);
    bufAfter=GWEN_Buffer_new(0, 128, 0, 1);

    _flagsToBuffer(j->flags, bufBefore);
    _flagsToBuffer(f, bufAfter);

    DBG_INFO(AQHBCI_LOGDOMAIN,
             "Changing flags of job \"%s\" to %08x: %s, was %08x: %s",
             j->name,
             f, GWEN_Buffer_GetStart(bufAfter),
             j->flags, GWEN_Buffer_GetStart(bufBefore));
    AB_Banking_LogMsgForJobId(AH_Job_GetBankingApi(j), AH_Job_GetId(j),
                              "Changing flags to %08x: %s (was %08x: %s)",
                              j->flags, GWEN_Buffer_GetStart(bufBefore),
                              f, GWEN_Buffer_GetStart(bufAfter));
    GWEN_Buffer_free(bufAfter);
    GWEN_Buffer_free(bufBefore);
    j->flags=f;
  }
}



void AH_Job_AddFlags(AH_JOB *j, uint32_t f)
{
  AH_Job_SetFlags(j, j->flags|f);
}



void AH_Job_SubFlags(AH_JOB *j, uint32_t f)
{
  AH_Job_SetFlags(j, j->flags&~f);
}



GWEN_DB_NODE *AH_Job_GetParams(const AH_JOB *j)
{
  assert(j);
  assert(j->usage);
  return j->jobParams;
}



GWEN_DB_NODE *AH_Job_GetArguments(const AH_JOB *j)
{
  assert(j);
  assert(j->usage);
  return j->jobArguments;
}



GWEN_DB_NODE *AH_Job_GetResponses(const AH_JOB *j)
{
  assert(j);
  assert(j->usage);
  return j->jobResponses;
}



uint32_t AH_Job_GetFirstSegment(const AH_JOB *j)
{
  assert(j);
  assert(j->usage);
  return j->firstSegment;
}



void AH_Job_SetFirstSegment(AH_JOB *j, uint32_t i)
{
  assert(j);
  assert(j->usage);
  j->firstSegment=i;
}



uint32_t AH_Job_GetLastSegment(const AH_JOB *j)
{
  assert(j);
  assert(j->usage);
  return j->lastSegment;
}



void AH_Job_SetLastSegment(AH_JOB *j, uint32_t i)
{
  assert(j);
  assert(j->usage);
  j->lastSegment=i;
}



int AH_Job_HasSegment(const AH_JOB *j, int seg)
{
  assert(j);
  assert(j->usage);
  DBG_DEBUG(AQHBCI_LOGDOMAIN, "Job \"%s\" checked for %d: first=%d, last=%d",
            j->name, seg,  j->firstSegment, j->lastSegment);
  return (seg<=j->lastSegment && seg>=j->firstSegment);
}



void AH_Job_AddResponse(AH_JOB *j, GWEN_DB_NODE *db)
{
  assert(j);
  assert(j->usage);
  GWEN_DB_AddGroup(j->jobResponses, db);
}



AH_JOB_STATUS AH_Job_GetStatus(const AH_JOB *j)
{
  assert(j);
  assert(j->usage);
  return j->status;
}



void AH_Job_SetStatus(AH_JOB *j, AH_JOB_STATUS st)
{
  assert(j);
  assert(j->usage);
  if (j->status!=st) {
    GWEN_BUFFER *lbuf;

    lbuf=GWEN_Buffer_new(0, 64, 0, 1);
    DBG_INFO(AQHBCI_LOGDOMAIN,
             "Changing status of job \"%s\" from \"%s\" (%d) to \"%s\" (%d)",
             j->name,
             AH_Job_StatusName(j->status), j->status,
             AH_Job_StatusName(st), st);
    GWEN_Buffer_AppendString(lbuf, "Status changed from \"");
    GWEN_Buffer_AppendString(lbuf, AH_Job_StatusName(j->status));
    GWEN_Buffer_AppendString(lbuf, "\" to \"");
    GWEN_Buffer_AppendString(lbuf, AH_Job_StatusName(st));
    GWEN_Buffer_AppendString(lbuf, "\"");

    AH_Job_Log(j, GWEN_LoggerLevel_Info, GWEN_Buffer_GetStart(lbuf));
    GWEN_Buffer_free(lbuf);

    AB_Banking_LogMsgForJobId(AH_Job_GetBankingApi(j), AH_Job_GetId(j),
                              "Changing status from \"%s\" (%d) to \"%s\" (%d)",
                              AH_Job_StatusName(j->status), j->status,
                              AH_Job_StatusName(st), st);

    j->status=st;

    /* set status to original command */
    if (j->commandList) {
      AB_TRANSACTION_LIST2_ITERATOR *jit;

      jit=AB_Transaction_List2_First(j->commandList);
      if (jit) {
        AB_TRANSACTION *t;
        AB_TRANSACTION_STATUS ts=AB_Transaction_StatusUnknown;

        switch (st) {
        case AH_JobStatusUnknown:
          ts=AB_Transaction_StatusUnknown;
          break;
        case AH_JobStatusToDo:
          ts=AB_Transaction_StatusEnqueued;
          break;
        case AH_JobStatusEnqueued:
          ts=AB_Transaction_StatusEnqueued;
          break;
        case AH_JobStatusEncoded:
          ts=AB_Transaction_StatusSending;
          break;
        case AH_JobStatusSent:
          ts=AB_Transaction_StatusSending;
          break;
        case AH_JobStatusAnswered:
          ts=AB_Transaction_StatusSending;
          break;
        case AH_JobStatusError:
          ts=AB_Transaction_StatusError;
          break;

        case AH_JobStatusAll:
          ts=AB_Transaction_StatusUnknown;
          break;
        }

        t=AB_Transaction_List2Iterator_Data(jit);
        while (t) {
          AB_Banking_LogMsgForJobId(AH_Job_GetBankingApi(j),
                                    AB_Transaction_GetUniqueId(t),
                                    "Changing command status to \"%s\" (%d)",
                                    AB_Transaction_Status_toString(ts), ts);
          AB_Transaction_SetStatus(t, ts);
          t=AB_Transaction_List2Iterator_Next(jit);
        }
        AB_Transaction_List2Iterator_free(jit);
      } /* if (jit) */
    } /* if (j->commandList) */
  }
}



void AH_Job_AddSigner(AH_JOB *j, const char *s)
{
  GWEN_BUFFER *lbuf;

  assert(j);
  assert(j->usage);
  assert(s);

  lbuf=GWEN_Buffer_new(0, 128, 0, 1);
  if (!GWEN_StringList_AppendString(j->signers, s, 0, 1)) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "Signer \"%s\" already in list", s);
    GWEN_Buffer_AppendString(lbuf, "Signer \"");
    GWEN_Text_EscapeToBufferTolerant(s, lbuf);
    GWEN_Buffer_AppendString(lbuf, "\" already in list");
    AH_Job_Log(j, GWEN_LoggerLevel_Warning,
               GWEN_Buffer_GetStart(lbuf));
  }
  else {
    GWEN_Buffer_AppendString(lbuf, "Signer \"");
    GWEN_Text_EscapeToBufferTolerant(s, lbuf);
    GWEN_Buffer_AppendString(lbuf, "\" added");
    AH_Job_Log(j, GWEN_LoggerLevel_Info,
               GWEN_Buffer_GetStart(lbuf));
    AB_Banking_LogMsgForJobId(AH_Job_GetBankingApi(j), AH_Job_GetId(j), "Adding signer \"%s\"", s?s:"<empty>");
  }
  GWEN_Buffer_free(lbuf);
  AH_Job_AddFlags(j, AH_JOB_FLAGS_SIGN);
}



int AH_Job_AddSigners(AH_JOB *j, const GWEN_STRINGLIST *sl)
{
  int sCount=0;

  if (sl) {
    GWEN_STRINGLISTENTRY *se;

    se=GWEN_StringList_FirstEntry(sl);
    while (se) {
      AH_Job_AddSigner(j, GWEN_StringListEntry_Data(se));
      sCount++;
      se=GWEN_StringListEntry_Next(se);
    } /* while */
  }

  return sCount;
}



AB_USER *AH_Job_GetUser(const AH_JOB *j)
{
  assert(j);
  assert(j->usage);
  return j->user;
}



const GWEN_STRINGLIST *AH_Job_GetSigners(const AH_JOB *j)
{
  assert(j);
  assert(j->usage);
  return j->signers;
}



GWEN_XMLNODE *AH_Job_GetXmlNode(const AH_JOB *j)
{
  assert(j);
  assert(j->usage);
  if (AH_Job_GetFlags(j) & AH_JOB_FLAGS_MULTIMSG) {
    DBG_DEBUG(AQHBCI_LOGDOMAIN,
              "Multi message node, returning current message node");
    return j->msgNode;
  }
  return j->xmlNode;
}



unsigned int AH_Job_GetMsgNum(const AH_JOB *j)
{
  assert(j);
  assert(j->usage);
  return j->msgNum;
}



const char *AH_Job_GetDialogId(const AH_JOB *j)
{
  assert(j);
  assert(j->usage);
  return j->dialogId;
}



void AH_Job_SetMsgNum(AH_JOB *j, unsigned int i)
{
  assert(j);
  assert(j->usage);
  j->msgNum=i;
}



void AH_Job_SetDialogId(AH_JOB *j, const char *s)
{
  assert(j);
  assert(j->usage);
  assert(s);

  free(j->dialogId);
  j->dialogId=strdup(s);
}



const char *AH_Job_StatusName(AH_JOB_STATUS st)
{
  switch (st) {
  case AH_JobStatusUnknown:
    return "unknown";
  case AH_JobStatusToDo:
    return "todo";
  case AH_JobStatusEnqueued:
    return "enqueued";
  case AH_JobStatusEncoded:
    return "encoded";
  case AH_JobStatusSent:
    return "sent";
  case AH_JobStatusAnswered:
    return "answered";
  case AH_JobStatusError:
    return "error";

  case AH_JobStatusAll:
    return "any";
  default:
    return "?";
  }
}


int AH_Job_HasWarnings(const AH_JOB *j)
{
  assert(j);
  assert(j->usage);
  return (AH_Job_GetFlags(j) & AH_JOB_FLAGS_HASWARNINGS);
}



int AH_Job_HasErrors(const AH_JOB *j)
{
  assert(j);
  assert(j->usage);
  return
    (j->status==AH_JobStatusError) ||
    (AH_Job_GetFlags(j) & AH_JOB_FLAGS_HASERRORS);
}



void AH_Job_SampleResults(AH_JOB *j)
{
  GWEN_DB_NODE *dbCurr;

  assert(j);
  assert(j->usage);

  dbCurr=GWEN_DB_GetFirstGroup(j->jobResponses);
  while (dbCurr) {
    GWEN_DB_NODE *dbResults;

    dbResults=GWEN_DB_GetGroup(dbCurr, GWEN_PATH_FLAGS_NAMEMUSTEXIST,
                               "data/SegResult");
    if (dbResults) {
      GWEN_DB_NODE *dbRes;

      dbRes=GWEN_DB_GetFirstGroup(dbResults);
      while (dbRes) {
        if (strcasecmp(GWEN_DB_GroupName(dbRes), "result")==0) {
          AH_RESULT *r;
          int code;
          const char *text;

          code=GWEN_DB_GetIntValue(dbRes, "resultcode", 0, 0);
          text=GWEN_DB_GetCharValue(dbRes, "text", 0, 0);
          if (code) {
            GWEN_BUFFER *lbuf;
            char numbuf[32];
            GWEN_LOGGER_LEVEL ll;

            if (code>=9000)
              ll=GWEN_LoggerLevel_Error;
            else if (code>=3000 && code!=3920)
              ll=GWEN_LoggerLevel_Warning;
            else
              ll=GWEN_LoggerLevel_Info;
            lbuf=GWEN_Buffer_new(0, 128, 0, 1);
            GWEN_Buffer_AppendString(lbuf, "SegResult: ");
            snprintf(numbuf, sizeof(numbuf), "%d", code);
            GWEN_Buffer_AppendString(lbuf, numbuf);
            if (text) {
              GWEN_Buffer_AppendString(lbuf, "(");
              GWEN_Buffer_AppendString(lbuf, text);
              GWEN_Buffer_AppendString(lbuf, ")");
            }
            AH_Job_Log(j, ll,
                       GWEN_Buffer_GetStart(lbuf));
            GWEN_Buffer_free(lbuf);
            AB_Banking_LogMsgForJobId(AH_Job_GetBankingApi(j), AH_Job_GetId(j),
                                      "SegResult: %d (%s)",
                                      code, text?text:"<empty>");
          }

          /* found a result */
          r=AH_Result_new(code,
                          text,
                          GWEN_DB_GetCharValue(dbRes, "ref", 0, 0),
                          GWEN_DB_GetCharValue(dbRes, "param", 0, 0),
                          0);
          AH_Result_List_Add(r, j->segResults);

          DBG_DEBUG(AQHBCI_LOGDOMAIN, "Segment result:");
          if (GWEN_Logger_GetLevel(0)>=GWEN_LoggerLevel_Debug)
            AH_Result_Dump(r, stderr, 4);

          /* check result */
          if (code>=9000)
            AH_Job_AddFlags(j, AH_JOB_FLAGS_HASERRORS);
          else if (code>=3000 && code<4000)
            AH_Job_AddFlags(j, AH_JOB_FLAGS_HASWARNINGS);
        } /* if result */
        dbRes=GWEN_DB_GetNextGroup(dbRes);
      } /* while */
    } /* if segResult */
    else {
      dbResults=GWEN_DB_GetGroup(dbCurr, GWEN_PATH_FLAGS_NAMEMUSTEXIST,
                                 "data/MsgResult");
      if (dbResults) {
        GWEN_DB_NODE *dbRes;

        dbRes=GWEN_DB_GetFirstGroup(dbResults);
        while (dbRes) {
          if (strcasecmp(GWEN_DB_GroupName(dbRes), "result")==0) {
            AH_RESULT *r;
            int code;
            const char *text;

            code=GWEN_DB_GetIntValue(dbRes, "resultcode", 0, 0);
            text=GWEN_DB_GetCharValue(dbRes, "text", 0, 0);
            if (code) {
              GWEN_BUFFER *lbuf;
              char numbuf[32];
              GWEN_LOGGER_LEVEL ll;

              if (code>=9000)
                ll=GWEN_LoggerLevel_Error;
              else if (code>=3000)
                ll=GWEN_LoggerLevel_Warning;
              else
                ll=GWEN_LoggerLevel_Info;
              lbuf=GWEN_Buffer_new(0, 128, 0, 1);
              GWEN_Buffer_AppendString(lbuf, "MsgResult: ");
              snprintf(numbuf, sizeof(numbuf), "%d", code);
              GWEN_Buffer_AppendString(lbuf, numbuf);
              if (text) {
                GWEN_Buffer_AppendString(lbuf, "(");
                GWEN_Buffer_AppendString(lbuf, text);
                GWEN_Buffer_AppendString(lbuf, ")");
              }
              AH_Job_Log(j, ll,
                         GWEN_Buffer_GetStart(lbuf));
              GWEN_Buffer_free(lbuf);
              AB_Banking_LogMsgForJobId(AH_Job_GetBankingApi(j), AH_Job_GetId(j),
                                        "MsgResult: %d (%s)",
                                        code, text?text:"<empty>");
            }

            /* found a result */
            r=AH_Result_new(code,
                            text,
                            GWEN_DB_GetCharValue(dbRes, "ref", 0, 0),
                            GWEN_DB_GetCharValue(dbRes, "param", 0, 0),
                            1);
            AH_Result_List_Add(r, j->msgResults);
            DBG_DEBUG(AQHBCI_LOGDOMAIN, "Message result:");
            if (GWEN_Logger_GetLevel(0)>=GWEN_LoggerLevel_Debug)
              AH_Result_Dump(r, stderr, 4);

            /* check result */
            if (code>=9000) {
              /* FIXME: Maybe disable here, let only the segment results
               * influence the error flags */
              AH_Job_AddFlags(j, AH_JOB_FLAGS_HASERRORS);
            }
            else if (code>=3000 && code<4000)
              AH_Job_AddFlags(j, AH_JOB_FLAGS_HASWARNINGS);
          } /* if result */
          dbRes=GWEN_DB_GetNextGroup(dbRes);
        } /* while */
      } /* if msgResult */
    }
    dbCurr=GWEN_DB_GetNextGroup(dbCurr);
  } /* while */

}



const char *AH_Job_GetDescription(const AH_JOB *j)
{
  assert(j);
  assert(j->usage);
  if (j->description)
    return j->description;
  return j->name;
}



void AH_Job_Dump(const AH_JOB *j, FILE *f, unsigned int insert)
{
  uint32_t k;

  for (k=0; k<insert; k++)
    fprintf(f, " ");
  fprintf(f, "Job:\n");

  for (k=0; k<insert; k++)
    fprintf(f, " ");
  fprintf(f, "Name          : %s\n", j->name);

  for (k=0; k<insert; k++)
    fprintf(f, " ");
  fprintf(f, "Code          : %s\n", (j->code)?(j->code):"(empty)");

  for (k=0; k<insert; k++)
    fprintf(f, " ");
  fprintf(f, "SegVer        : %d\n", j->segmentVersion);

  for (k=0; k<insert; k++)
    fprintf(f, " ");
  fprintf(f, "FirstSegment  : %d\n", j->firstSegment);

  for (k=0; k<insert; k++)
    fprintf(f, " ");
  fprintf(f, "LasttSegment  : %d\n", j->lastSegment);

  for (k=0; k<insert; k++)
    fprintf(f, " ");
  fprintf(f, "ChallengeClass: %d\n", j->challengeClass);

  for (k=0; k<insert; k++)
    fprintf(f, " ");
  fprintf(f, "MinSigs       : %d\n", j->minSigs);

  for (k=0; k<insert; k++)
    fprintf(f, " ");
  fprintf(f, "SecProfile    : %d\n", j->secProfile);

  for (k=0; k<insert; k++)
    fprintf(f, " ");
  fprintf(f, "SecClass      : %d\n", j->secClass);

  for (k=0; k<insert; k++)
    fprintf(f, " ");
  fprintf(f, "JobsPerMsg    : %d\n", j->jobsPerMsg);


  for (k=0; k<insert; k++)
    fprintf(f, " ");
  fprintf(f, "Status        : %s (%d)\n", AH_Job_StatusName(j->status), j->status);

  for (k=0; k<insert; k++)
    fprintf(f, " ");
  fprintf(f, "Msgnum        : %d\n", j->msgNum);

  for (k=0; k<insert; k++)
    fprintf(f, " ");
  fprintf(f, "DialogId      : %s\n", j->dialogId);

  for (k=0; k<insert; k++)
    fprintf(f, " ");
  fprintf(f, "Owner         : %s\n", AB_User_GetCustomerId(j->user));

  for (k=0; k<insert; k++)
    fprintf(f, " ");
  fprintf(f, "MaxTransfers  : %d\n", j->maxTransfers);

  for (k=0; k<insert; k++)
    fprintf(f, " ");
  fprintf(f, "TransferCount : %d\n", j->transferCount);

  for (k=0; k<insert; k++)
    fprintf(f, " ");
  fprintf(f, "SupportedCmd  : %s\n", AB_Transaction_Command_toString(j->supportedCommand));

  for (k=0; k<insert; k++)
    fprintf(f, " ");
  fprintf(f, "Flags: %08x ( ", j->flags);
  if (j->flags & AH_JOB_FLAGS_IGNOREACCOUNTS)
    fprintf(f, "IGNOREACCOUNTS ");
  if (j->flags & AH_JOB_FLAGS_SIGNSEQONE)
    fprintf(f, "SIGNSEQONE ");
  if (j->flags & AH_JOB_FLAGS_IGNORE_ERROR)
    fprintf(f, "IGNORE_ERROR ");
  if (j->flags & AH_JOB_FLAGS_NOITAN)
    fprintf(f, "NOITAN ");
  if (j->flags & AH_JOB_FLAGS_TANUSED)
    fprintf(f, "TANUSED ");
  if (j->flags & AH_JOB_FLAGS_NOSYSID)
    fprintf(f, "NOSYSID ");
  if (j->flags & AH_JOB_FLAGS_NEEDCRYPT)
    fprintf(f, "NEEDCRYPT ");
  if (j->flags & AH_JOB_FLAGS_NEEDSIGN)
    fprintf(f, "NEEDSIGN ");
  if (j->flags & AH_JOB_FLAGS_ATTACHABLE)
    fprintf(f, "ATTACHABLE ");
  if (j->flags & AH_JOB_FLAGS_SINGLE)
    fprintf(f, "SINGLE ");
  if (j->flags & AH_JOB_FLAGS_DLGJOB)
    fprintf(f, "DLGJOB ");
  if (j->flags & AH_JOB_FLAGS_CRYPT)
    fprintf(f, "CRYPT ");
  if (j->flags & AH_JOB_FLAGS_SIGN)
    fprintf(f, "SIGN ");
  if (j->flags & AH_JOB_FLAGS_MULTIMSG)
    fprintf(f, "MULTIMSG ");
  if (j->flags & AH_JOB_FLAGS_HASATTACHPOINT)
    fprintf(f, "HASATTACHPOINT ");
  if (j->flags & AH_JOB_FLAGS_HASMOREMSGS)
    fprintf(f, "HASMOREMSGS ");
  if (j->flags & AH_JOB_FLAGS_HASWARNINGS)
    fprintf(f, "HASWARNINGS ");
  if (j->flags & AH_JOB_FLAGS_HASERRORS)
    fprintf(f, "HASERRORS ");
  if (j->flags & AH_JOB_FLAGS_PROCESSED)
    fprintf(f, "PROCESSED ");
  if (j->flags & AH_JOB_FLAGS_COMMITTED)
    fprintf(f, "COMMITTED ");
  if (j->flags & AH_JOB_FLAGS_NEEDTAN)
    fprintf(f, "NEEDTAN ");
  if (j->flags & AH_JOB_FLAGS_OUTBOX)
    fprintf(f, "OUTBOX ");
  fprintf(f, ")\n");

  if (j->segResults) {
    AH_RESULT *r;

    for (k=0; k<insert; k++)
      fprintf(f, " ");
    fprintf(f, "Segment results:\n");
    r=AH_Result_List_First(j->segResults);
    while (r) {
      int code;
      const char *text;

      code=AH_Result_GetCode(r);
      text=AH_Result_GetText(r);
      for (k=0; k<insert+2; k++)
        fprintf(f, " ");
      fprintf(f, "%04d: %s\n", code, text?text:"<no text>");
      r=AH_Result_List_Next(r);
    }
  }

  if (j->jobResponses) {
    for (k=0; k<insert; k++)
      fprintf(f, " ");
    fprintf(f, "Response Data:\n");
    GWEN_DB_Dump(j->jobResponses, insert+2);
  }
}



void AH_Job_DumpShort(const AH_JOB *j, FILE *f, unsigned int insert)
{
  if (j) {
    uint32_t k;
    GWEN_BUFFER *dbuf;

    dbuf=GWEN_Buffer_new(0, 128, 0, 1);

    _flagsToBuffer(AH_Job_GetFlags(j), dbuf);

    for (k=0; k<insert; k++)
      fprintf(f, " ");
    fprintf(f, "- %s(%s)[%d] (%d-%d): %s(%d) [%s]\n",
            j->name, j->code, j->segmentVersion, j->firstSegment, j->lastSegment,
            AH_Job_StatusName(j->status), j->status,
            GWEN_Buffer_GetStart(dbuf));
    GWEN_Buffer_free(dbuf);
  }
  else {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "No job");
  }
}




int AH_Job_HasItanResult(const AH_JOB *j)
{

  return AH_Job_HasResultWithCode(j, 3920);
}



int AH_Job_HasResultWithCode(const AH_JOB *j, int wantedCode)
{
  GWEN_DB_NODE *dbCurr;

  assert(j);
  assert(j->usage);

  dbCurr=GWEN_DB_GetFirstGroup(j->jobResponses);
  while (dbCurr) {
    GWEN_DB_NODE *dbRd;

    dbRd=GWEN_DB_GetGroup(dbCurr, GWEN_PATH_FLAGS_NAMEMUSTEXIST, "data");
    if (dbRd) {
      dbRd=GWEN_DB_GetFirstGroup(dbRd);
    }
    if (dbRd) {
      const char *sGroupName;

      sGroupName=GWEN_DB_GroupName(dbRd);

      if (sGroupName && *sGroupName &&
          ((strcasecmp(sGroupName, "SegResult")==0) ||
           (strcasecmp(sGroupName, "MsgResult")==0))) {
        GWEN_DB_NODE *dbRes;

        dbRes=GWEN_DB_GetFirstGroup(dbRd);
        while (dbRes) {
          if (strcasecmp(GWEN_DB_GroupName(dbRes), "result")==0) {
            int code;

            code=GWEN_DB_GetIntValue(dbRes, "resultcode", 0, 0);
            DBG_DEBUG(AQHBCI_LOGDOMAIN, "Checking result code %d against %d", code, wantedCode);
            if (code==wantedCode) {
              return 1;
            }
          } /* if result */
          dbRes=GWEN_DB_GetNextGroup(dbRes);
        } /* while */
      }
    } /* if response data found */
    dbCurr=GWEN_DB_GetNextGroup(dbCurr);
  } /* while */

  return 0; /* no iTAN response */
}



AH_JOB *AH_Job__freeAll_cb(AH_JOB *j, void *userData)
{
  assert(j);
  assert(j->usage);
  AH_Job_free(j);
  return 0;
}



void AH_Job_List2_FreeAll(AH_JOB_LIST2 *jl)
{
  AH_Job_List2_ForEach(jl, AH_Job__freeAll_cb, 0);
  AH_Job_List2_free(jl);
}



AH_HBCI *AH_Job_GetHbci(const AH_JOB *j)
{
  assert(j);
  assert(j->usage);

  return AH_User_GetHbci(j->user);
}



AB_BANKING *AH_Job_GetBankingApi(const AH_JOB *j)
{
  AH_HBCI *hbci;

  assert(j);
  assert(j->usage);
  hbci=AH_Job_GetHbci(j);
  assert(hbci);
  return AH_HBCI_GetBankingApi(hbci);
}



AH_RESULT_LIST *AH_Job_GetSegResults(const AH_JOB *j)
{
  assert(j);
  assert(j->usage);
  return j->segResults;
}



AH_RESULT_LIST *AH_Job_GetMsgResults(const AH_JOB *j)
{
  assert(j);
  assert(j->usage);
  return j->msgResults;
}



const char *AH_Job_GetExpectedSigner(const AH_JOB *j)
{
  assert(j);
  assert(j->usage);
  return j->expectedSigner;
}



void AH_Job_SetExpectedSigner(AH_JOB *j, const char *s)
{
  assert(j);
  assert(j->usage);
  free(j->expectedSigner);
  if (s)
    j->expectedSigner=strdup(s);
  else
    j->expectedSigner=0;
}



const char *AH_Job_GetExpectedCrypter(const AH_JOB *j)
{
  assert(j);
  assert(j->usage);
  return j->expectedCrypter;
}



void AH_Job_SetExpectedCrypter(AH_JOB *j, const char *s)
{
  assert(j);
  assert(j->usage);
  free(j->expectedCrypter);
  if (s)
    j->expectedCrypter=strdup(s);
  else
    j->expectedCrypter=0;
}



const char *AH_Job_GetUsedTan(const AH_JOB *j)
{
  assert(j);
  assert(j->usage);
  return j->usedTan;
}



void AH_Job_SetUsedTan(AH_JOB *j, const char *s)
{
  assert(j);
  assert(j->usage);

  DBG_DEBUG(AQHBCI_LOGDOMAIN, "Changing TAN in job [%s](%08x) from [%s] to [%s]",
            j->name, j->id,
            (j->usedTan)?(j->usedTan):"(empty)",
            s?s:"(empty)");
  free(j->usedTan);
  if (s) {
    j->usedTan=strdup(s);
  }
  else
    j->usedTan=0;
}



void AH_Job_Log(AH_JOB *j, GWEN_LOGGER_LEVEL ll, const char *txt)
{
  char buffer[32];
  GWEN_TIME *ti;
  GWEN_BUFFER *lbuf;

  assert(j);

  lbuf=GWEN_Buffer_new(0, 128, 0, 1);
  snprintf(buffer, sizeof(buffer), "%02d", ll);
  GWEN_Buffer_AppendString(lbuf, buffer);
  GWEN_Buffer_AppendByte(lbuf, ':');
  ti=GWEN_CurrentTime();
  assert(ti);
  GWEN_Time_toString(ti, "YYYYMMDD:hhmmss:", lbuf);
  GWEN_Time_free(ti);
  GWEN_Text_EscapeToBufferTolerant(AH_PROVIDER_NAME, lbuf);
  GWEN_Buffer_AppendByte(lbuf, ':');
  GWEN_Text_EscapeToBufferTolerant(txt, lbuf);
  GWEN_StringList_AppendString(j->log,
                               GWEN_Buffer_GetStart(lbuf),
                               0, 0);
  GWEN_Buffer_free(lbuf);
}



const GWEN_STRINGLIST *AH_Job_GetLogs(const AH_JOB *j)
{
  assert(j);
  return j->log;
}



GWEN_STRINGLIST *AH_Job_GetChallengeParams(const AH_JOB *j)
{
  assert(j);
  return j->challengeParams;
}



void AH_Job_ClearChallengeParams(AH_JOB *j)
{
  assert(j);
  GWEN_StringList_Clear(j->challengeParams);
}



void AH_Job_AddChallengeParam(AH_JOB *j, const char *s)
{
  assert(j);
  GWEN_StringList_AppendString(j->challengeParams, s, 0, 0);
}



void AH_Job_ValueToChallengeString(const AB_VALUE *v, GWEN_BUFFER *buf)
{
  AB_Value_toHbciString(v, buf);
}



int AH_Job_GetTransferCount(AH_JOB *j)
{
  assert(j);
  return j->transferCount;
}



void AH_Job_IncTransferCount(AH_JOB *j)
{
  assert(j);
  j->transferCount++;
}



int AH_Job_GetMaxTransfers(AH_JOB *j)
{
  assert(j);
  return j->maxTransfers;
}



void AH_Job_SetMaxTransfers(AH_JOB *j, int i)
{
  assert(j);
  j->maxTransfers=i;
}



AB_TRANSACTION_LIST *AH_Job_GetTransferList(const AH_JOB *j)
{
  assert(j);
  return j->transferList;
}



AB_TRANSACTION *AH_Job_GetFirstTransfer(const AH_JOB *j)
{
  assert(j);
  if (j->transferList==NULL)
    return NULL;

  return AB_Transaction_List_First(j->transferList);
}



void AH_Job_AddTransfer(AH_JOB *j, AB_TRANSACTION *t)
{
  assert(j);
  if (j->transferList==NULL)
    j->transferList=AB_Transaction_List_new();

  AB_Transaction_List_Add(t, j->transferList);
  j->transferCount++;
}



AB_TRANSACTION_COMMAND AH_Job_GetSupportedCommand(const AH_JOB *j)
{
  assert(j);
  assert(j->usage);
  return j->supportedCommand;
}



void AH_Job_SetSupportedCommand(AH_JOB *j, AB_TRANSACTION_COMMAND tc)
{
  assert(j);
  assert(j->usage);
  j->supportedCommand=tc;
}



AB_PROVIDER *AH_Job_GetProvider(const AH_JOB *j)
{
  assert(j);
  assert(j->usage);
  return j->provider;
}



void AH_Job_AddCommand(AH_JOB *j, AB_TRANSACTION *t)
{
  assert(j);
  assert(j->usage);

  if (j->commandList==NULL)
    j->commandList=AB_Transaction_List2_new();
  AB_Transaction_List2_PushBack(j->commandList, t);
}



AB_TRANSACTION_LIST2 *AH_Job_GetCommandList(const AH_JOB *j)
{
  assert(j);
  assert(j->usage);

  return j->commandList;
}



AH_JOB *AH_Job_List_GetById(AH_JOB_LIST *jl, uint32_t id)
{
  if (jl) {
    AH_JOB *j;

    j=AH_Job_List_First(jl);
    while (j) {
      if (AH_Job_GetId(j)==id)
        return j;
      j=AH_Job_List_Next(j);
    }
  }

  return NULL;
}



void AH_Job_SetStatusOnCommands(AH_JOB *j, AB_TRANSACTION_STATUS status)
{
  AB_TRANSACTION_LIST2 *cmdList;

  assert(j);

  cmdList=AH_Job_GetCommandList(j);
  if (cmdList) {
    AB_TRANSACTION_LIST2_ITERATOR *it;

    it=AB_Transaction_List2_First(cmdList);
    if (it) {
      AB_TRANSACTION *t;

      t=AB_Transaction_List2Iterator_Data(it);
      while (t) {
        AB_Transaction_SetStatus(t, status);
        t=AB_Transaction_List2Iterator_Next(it);
      }
      AB_Transaction_List2Iterator_free(it);
    }
  }
}



char *AH_Job_GenerateIdFromDateTimeAndJobId(const AH_JOB *j, int runningNumber)
{
  GWEN_TIME *ti;
  int days, month, year;
  int hours, mins, secs;
  char *string31;
  int rv;

  ti=GWEN_CurrentTime();
  assert(ti);

  GWEN_Time_GetBrokenDownDate(ti, &days, &month, &year);
  GWEN_Time_GetBrokenDownTime(ti, &hours, &mins, &secs);
  GWEN_Time_free(ti);

  string31=(char *) malloc(31);
  /* YYYYMMDDhhmmssJJJJJJJJRRRRRRRR */
  rv=snprintf(string31, 31, "%04d%02d%02d%02d%02d%02d%08x%08x",
              year, month+1, days, hours, mins, secs, j->id, runningNumber);
  if (rv<0 || rv>30) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "Error on snprintf (%d)", rv);
    free(string31);
    return NULL;
  }

  return string31;
}



void _flagsToBuffer(uint32_t flags, GWEN_BUFFER *dbuf)
{
  if (flags==0)
    GWEN_Buffer_AppendString(dbuf, "<NONE> ");
  else if (flags & AH_JOB_FLAGS_ACKNOWLEDGE)
    GWEN_Buffer_AppendString(dbuf, "ACKNOWLEDGE ");
  else if (flags & AH_JOB_FLAGS_IGNOREACCOUNTS)
    GWEN_Buffer_AppendString(dbuf, "IGNOREACCOUNTS ");
  if (flags & AH_JOB_FLAGS_SIGNSEQONE)
    GWEN_Buffer_AppendString(dbuf, "SIGNSEQONE ");
  if (flags & AH_JOB_FLAGS_IGNORE_ERROR)
    GWEN_Buffer_AppendString(dbuf, "IGNORE_ERROR ");
  if (flags & AH_JOB_FLAGS_NOITAN)
    GWEN_Buffer_AppendString(dbuf, "NOITAN ");
  if (flags & AH_JOB_FLAGS_TANUSED)
    GWEN_Buffer_AppendString(dbuf, "TANUSED ");
  if (flags & AH_JOB_FLAGS_NOSYSID)
    GWEN_Buffer_AppendString(dbuf, "NOSYSID ");
  if (flags & AH_JOB_FLAGS_NEEDCRYPT)
    GWEN_Buffer_AppendString(dbuf, "NEEDCRYPT ");
  if (flags & AH_JOB_FLAGS_NEEDSIGN)
    GWEN_Buffer_AppendString(dbuf, "NEEDSIGN ");
  if (flags & AH_JOB_FLAGS_ATTACHABLE)
    GWEN_Buffer_AppendString(dbuf, "ATTACHABLE ");
  if (flags & AH_JOB_FLAGS_SINGLE)
    GWEN_Buffer_AppendString(dbuf, "SINGLE ");
  if (flags & AH_JOB_FLAGS_DLGJOB)
    GWEN_Buffer_AppendString(dbuf, "DLGJOB ");
  if (flags & AH_JOB_FLAGS_CRYPT)
    GWEN_Buffer_AppendString(dbuf, "CRYPT ");
  if (flags & AH_JOB_FLAGS_SIGN)
    GWEN_Buffer_AppendString(dbuf, "SIGN ");
  if (flags & AH_JOB_FLAGS_MULTIMSG)
    GWEN_Buffer_AppendString(dbuf, "MULTIMSG ");
  if (flags & AH_JOB_FLAGS_HASATTACHPOINT)
    GWEN_Buffer_AppendString(dbuf, "HASATTACHPOINT ");
  if (flags & AH_JOB_FLAGS_HASMOREMSGS)
    GWEN_Buffer_AppendString(dbuf, "HASMOREMSGS ");
  if (flags & AH_JOB_FLAGS_HASWARNINGS)
    GWEN_Buffer_AppendString(dbuf, "HASWARNINGS ");
  if (flags & AH_JOB_FLAGS_HASERRORS)
    GWEN_Buffer_AppendString(dbuf, "HASERRORS ");
  if (flags & AH_JOB_FLAGS_PROCESSED)
    GWEN_Buffer_AppendString(dbuf, "PROCESSED ");
  if (flags & AH_JOB_FLAGS_COMMITTED)
    GWEN_Buffer_AppendString(dbuf, "COMMITTED ");
  if (flags & AH_JOB_FLAGS_NEEDTAN)
    GWEN_Buffer_AppendString(dbuf, "NEEDTAN ");
  if (flags & AH_JOB_FLAGS_OUTBOX)
    GWEN_Buffer_AppendString(dbuf, "OUTBOX ");
}


#include "job_new.c"
#include "job_virtual.c"


