/***************************************************************************
    begin       : Mon Mar 01 2004
    copyright   : (C) 2020 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


/* file included by job.c */



/* ------------------------------------------------------------------------------------------------
 * forward declarations
 * ------------------------------------------------------------------------------------------------
 */


static GWEN_XMLNODE *_jobGetJobNode(const AH_JOB *j, int jobVersion);
static int _jobGetBpdParamsForVersion(const AH_JOB *j, const char *paramName, int jobVersion);
static GWEN_DB_NODE *_jobGetUpdJob(const AH_JOB *j, const AB_ACCOUNT *a);
static GWEN_DB_NODE *_jobGetBpdPinTanParams(const AH_JOB *j);

static void _jobReadFromDescriptorNode(AH_JOB *j, GWEN_XMLNODE *jobNode);
static void _jobReadFromBpdParamsNode(AH_JOB *j, GWEN_DB_NODE *jobBPD);
static void _jobReadFromUpdNode(AH_JOB *j, GWEN_DB_NODE *jobUPD);
static void _jobReadSepaDescriptors(AH_JOB *j);




/* ------------------------------------------------------------------------------------------------
 * code
 * ------------------------------------------------------------------------------------------------
 */

AH_JOB *AH_Job_new(const char *name,
                   AB_PROVIDER *pro,
                   AB_USER *u,
                   AB_ACCOUNT *acc,
                   int jobVersion)
{
  AH_JOB *j;
  GWEN_MSGENGINE *e;
  const char *paramName=NULL;

  assert(name);
  assert(u);

  GWEN_NEW_OBJECT(AH_JOB, j);
  j->usage=1;
  GWEN_LIST_INIT(AH_JOB, j);
  GWEN_INHERIT_INIT(AH_JOB, j);

  j->name=strdup(name);
  j->user=u;
  j->provider=pro;
  j->signers=GWEN_StringList_new();
  j->log=GWEN_StringList_new();
  j->challengeParams=GWEN_StringList_new();
  j->jobParams=GWEN_DB_Group_new("jobParams");
  j->jobArguments=GWEN_DB_Group_new("jobArguments");
  j->jobResponses=GWEN_DB_Group_new("jobResponses");
  j->segResults=AH_Result_List_new();
  j->msgResults=AH_Result_List_new();
  j->messages=AB_Message_List_new();

  AH_Job_AddFlags(j, AH_JOB_FLAGS_HASMOREMSGS);
  if (AH_User_GetCryptMode(u)==AH_CryptMode_Pintan)
    /* always make jobs single when in PIN/TAN mode */
    AH_Job_AddFlags(j, AH_JOB_FLAGS_SINGLE);

  /* setup message engine */
  e=AH_User_GetMsgEngine(u);
  assert(e);
  j->msgEngine=e;
  GWEN_MsgEngine_Attach(e); /* just to make sure the XMLNode is not freed before this job is */
  if (AH_User_GetHbciVersion(u)==0)
    GWEN_MsgEngine_SetProtocolVersion(e, 300);
  else
    GWEN_MsgEngine_SetProtocolVersion(e, AH_User_GetHbciVersion(u));
  GWEN_MsgEngine_SetMode(e, AH_CryptMode_toString(AH_User_GetCryptMode(u)));


  /* sample some info from job description node */
  if (1) {
    GWEN_XMLNODE *jobNode=NULL;

    /* get job descriptor node for selected (or highest) version */
    DBG_INFO(AQHBCI_LOGDOMAIN, "Reading job description from XML files");
    jobNode=_jobGetJobNode(j, jobVersion);
    if (!jobNode) {
      DBG_INFO(AQHBCI_LOGDOMAIN, "Job \"%s\" not supported by local XML files", name);
      AH_Job_free(j);
      return NULL;
    }
    j->xmlNode=jobNode;
    paramName=GWEN_XMLNode_GetProperty(jobNode, "params", NULL);

    _jobReadFromDescriptorNode(j, jobNode);
  }

  /* sample PinTan params */
  if (1) {
    GWEN_DB_NODE *jobPinTan;

    jobPinTan=_jobGetBpdPinTanParams(j);
    if (jobPinTan) {
      /* sample flag NEEDTAN */
      AH_Job_AddFlags(j, (GWEN_DB_GetIntValue(jobPinTan, "needTan", 0, 0)!=0)?AH_JOB_FLAGS_NEEDTAN:0);
    }
  }

  /* sample some info from BPD */
  if (paramName && *paramName) {
    GWEN_DB_NODE *jobBPD;

    DBG_INFO(AQHBCI_LOGDOMAIN, "Reading info from BPD job (if any)");
    jobBPD=AH_User_GetBpdJobForParamNameAndVersion(j->user, paramName, j->segmentVersion);
    if (jobBPD)
      _jobReadFromBpdParamsNode(j, jobBPD);
  }

  /* sample some info from UPD */
  if (1) {
    GWEN_DB_NODE *jobUPD;

    DBG_INFO(AQHBCI_LOGDOMAIN, "Reading info from UPD job (if any)");
    jobUPD=_jobGetUpdJob(j, acc);
    if (jobUPD)
      _jobReadFromUpdNode(j, jobUPD);
  }

  /* check BPD for job specific SEPA descriptors */
  DBG_INFO(AQHBCI_LOGDOMAIN, "Reading SEPA format descriptors (if any)");
  _jobReadSepaDescriptors(j);

  /* done */
  if (GWEN_Logger_GetLevel(AQHBCI_LOGDOMAIN)>=GWEN_LoggerLevel_Debug) {
    DBG_DEBUG(AQHBCI_LOGDOMAIN, "Created this job:");
    AH_Job_Dump(j, stderr, 2);
  }

  AH_Job_Log(j, GWEN_LoggerLevel_Info, "HBCI-Job created");

  DBG_INFO(AQHBCI_LOGDOMAIN, "%s: Job created", j->name);
  return j;
}







GWEN_XMLNODE *_jobGetJobNode(const AH_JOB *j, int jobVersion)
{
  GWEN_XMLNODE *node;
  const char *paramName;
  int needsBPD;
  int rv;
  int realJobVersion=0;

  node=GWEN_MsgEngine_FindNodeByProperty(j->msgEngine, "JOB", "id", 0, j->name);
  if (!node) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "Job \"%s\" not supported by local XML files", j->name);
    return NULL;
  }

  /* preset */
  realJobVersion=GWEN_XMLNode_GetIntProperty(node, "version", 0);

  paramName=GWEN_XMLNode_GetProperty(node, "params", NULL);
  needsBPD=GWEN_XMLNode_GetIntProperty(node, "needbpd", 0)!=0;
  if (paramName && *paramName) {
    rv=_jobGetBpdParamsForVersion(j, paramName, jobVersion);
    if (rv<0) {
      DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
      if (needsBPD) {
        DBG_INFO(AQHBCI_LOGDOMAIN, "No BPD job found when needed");
        return NULL;
      }
    }
    else
      realJobVersion=rv;
  }

  if (realJobVersion<1) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "Could not determine real job version to use");
    return NULL;
  }

  node=GWEN_MsgEngine_FindNodeByProperty(j->msgEngine, "JOB", "id", realJobVersion, j->name);
  if (node==NULL) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "Job node \"%s\"[%d] not found", j->name, realJobVersion);
    return NULL;
  }

  return node;
}



GWEN_DB_NODE *_jobGetBpdPinTanParams(const AH_JOB *j)
{
  if (j->code) {
    const AH_BPD *bpd;
    GWEN_DB_NODE *bpdgrp;
    GWEN_DB_NODE *jobPinTan;

    DBG_INFO(AQHBCI_LOGDOMAIN, "Searching BPD PinTan params for job \"%s\" (%s)", j->code, j->name);

    bpd=AH_User_GetBpd(j->user);
    if (!bpd) {
      DBG_ERROR(AQHBCI_LOGDOMAIN, "No BPD");
      return NULL;
    }

    bpdgrp=AH_Bpd_GetBpdJobs(bpd, AH_User_GetHbciVersion(j->user));
    if (bpdgrp==NULL) {
      DBG_ERROR(AQHBCI_LOGDOMAIN, "No BPD jobs in BPD");
      return NULL;
    }

    jobPinTan=GWEN_DB_GetGroup(bpdgrp, GWEN_PATH_FLAGS_NAMEMUSTEXIST, j->code);
    if (jobPinTan)
      return jobPinTan;
  }

  return NULL;
}



int _jobGetBpdParamsForVersion(const AH_JOB *j, const char *paramName, int jobVersion)
{
  const AH_BPD *bpd;
  GWEN_DB_NODE *bpdgrp;
  GWEN_DB_NODE *jobBPD;
  int highestVersion;

  DBG_INFO(AQHBCI_LOGDOMAIN, "Searching BPD job \"%s\" for Job \"%s\" (version %d)", paramName, j->name, jobVersion);

  bpd=AH_User_GetBpd(j->user);
  if (!bpd) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "No BPD");
    return GWEN_ERROR_GENERIC;
  }

  bpdgrp=AH_Bpd_GetBpdJobs(bpd, AH_User_GetHbciVersion(j->user));
  if (bpdgrp==NULL) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "No BPD jobs in BPD");
    return GWEN_ERROR_GENERIC;
  }

  /* get BPD job */
  jobBPD=GWEN_DB_GetGroup(bpdgrp, GWEN_PATH_FLAGS_NAMEMUSTEXIST, paramName);
  if (jobBPD) {
    /* children are one group per version */
    jobBPD=GWEN_DB_GetFirstGroup(jobBPD);
  }

  /* check for a job for which we have a BPD */
  highestVersion=-1;

  if (jobVersion) {
    /* a job version has been selected from outside, look for
     * the BPD of that particular version */
    while (jobBPD) {
      int version;

      /* get version from BPD */
      version=atoi(GWEN_DB_GroupName(jobBPD));
      DBG_INFO(AQHBCI_LOGDOMAIN, "Checking Job %s (%d)", j->name, version);
      if (version==jobVersion) {
        GWEN_XMLNODE *node;

        /* now get the correct version of the JOB */
        DBG_INFO(AQHBCI_LOGDOMAIN, "Checking whether job %s (%d) can be instantiated", j->name, version);
        node=GWEN_MsgEngine_FindNodeByProperty(j->msgEngine, "JOB", "id", version, j->name);
        if (node) {
          DBG_INFO(AQHBCI_LOGDOMAIN, "Found BPD job");
          highestVersion=version;
          break;
        }
      }
      jobBPD=GWEN_DB_GetNextGroup(jobBPD);
    } /* while */
  }
  else {
    while (jobBPD) {
      int version;

      /* get version from BPD */
      version=atoi(GWEN_DB_GroupName(jobBPD));
      DBG_INFO(AQHBCI_LOGDOMAIN, "Checking Job %s (%d)", j->name, version);
      if (version>highestVersion) {
        GWEN_XMLNODE *node;

        /* now get the correct version of the JOB */
        DBG_INFO(AQHBCI_LOGDOMAIN, "Checking whether job %s (%d) can be instantiated", j->name, version);
        node=GWEN_MsgEngine_FindNodeByProperty(j->msgEngine, "JOB", "id", version, j->name);
        if (node) {
          DBG_INFO(AQHBCI_LOGDOMAIN, "Found BPD job candidate version %d", version);
          highestVersion=version;
        }
      }
      jobBPD=GWEN_DB_GetNextGroup(jobBPD);
    } /* while */
  }

  if (highestVersion<1) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "No matching BPD job found for job \"%s\"(%d)", j->name, jobVersion);
    return GWEN_ERROR_NOT_FOUND;
  }

  return highestVersion;
}



GWEN_DB_NODE *_jobGetUpdJob(const AH_JOB *j, const AB_ACCOUNT *a)
{
  if (j && a && j->code && *(j->code)) {
    GWEN_DB_NODE *updgroup;
    GWEN_DB_NODE *updnode=NULL;

    updgroup=AH_User_GetUpdForAccount(j->user, a);
    if (updgroup) {
      DBG_NOTICE(AQHBCI_LOGDOMAIN, "Code is \"%s\"", j->code);
      updnode=GWEN_DB_GetFirstGroup(updgroup);
      while (updnode) {
        if (strcasecmp(GWEN_DB_GetCharValue(updnode, "job", 0, ""), j->code)==0)
          return updnode;
        updnode=GWEN_DB_GetNextGroup(updnode);
      } /* while */
    } /* if updgroup for the given account found */
  } /* if account */
  return NULL;
}



void _jobReadFromDescriptorNode(AH_JOB *j, GWEN_XMLNODE *jobNode)
{
  GWEN_XMLNODE *msgNode;
  GWEN_XMLNODE *descrNode;
  const char *s;

  j->xmlNode=jobNode;

  /* get some properties */
  j->segmentVersion=GWEN_XMLNode_GetIntProperty(jobNode, "version", 0);

  s=GWEN_XMLNode_GetProperty(jobNode, "response", NULL);
  AH_Job_SetResponseName(j, s);

  s=GWEN_XMLNode_GetProperty(jobNode, "code", NULL);
  AH_Job_SetCode(j, s);

  /* sample flags from XML file */
  if (GWEN_XMLNode_GetIntProperty(jobNode, "dlg", 0)!=0) {
    AH_Job_AddFlags(j, AH_JOB_FLAGS_DLGJOB);
    AH_Job_AddFlags(j, AH_JOB_FLAGS_SINGLE);
  }
  AH_Job_AddFlags(j, (GWEN_XMLNode_GetIntProperty(jobNode, "attachable", 0)!=0)?AH_JOB_FLAGS_ATTACHABLE:0);
  AH_Job_AddFlags(j, (GWEN_XMLNode_GetIntProperty(jobNode, "single", 0)!=0)?AH_JOB_FLAGS_SINGLE:0);
  AH_Job_AddFlags(j, (GWEN_XMLNode_GetIntProperty(jobNode, "ignoreAccounts", 0)!=0)?AH_JOB_FLAGS_IGNOREACCOUNTS:0);

  /* get description */
  descrNode=GWEN_XMLNode_FindFirstTag(jobNode, "DESCR", 0, 0);
  if (descrNode) {
    GWEN_BUFFER *descrBuf;
    GWEN_XMLNODE *dn;

    descrBuf=GWEN_Buffer_new(0, 64, 0, 1);
    dn=GWEN_XMLNode_GetFirstData(descrNode);
    while (dn) {
      const char *d;

      d=GWEN_XMLNode_GetData(dn);
      if (d) {
        GWEN_Buffer_AppendString(descrBuf, d);
      }
      dn=GWEN_XMLNode_GetNextData(dn);
    } /* while */
    if (GWEN_Buffer_GetUsedBytes(descrBuf)) {
      j->description=strdup(GWEN_Buffer_GetStart(descrBuf));
    }
    GWEN_Buffer_free(descrBuf);
  } /* if there is a description */


  /* check for multi message job */
  msgNode=GWEN_XMLNode_FindFirstTag(jobNode, "MESSAGE", 0, 0);
  if (msgNode) {
    /* we have <MESSAGE> nodes, so this is not a simple case */
    DBG_INFO(AQHBCI_LOGDOMAIN, "Multi message job");
    /* GWEN_XMLNode_Dump(msgNode, stderr, 2); */
    AH_Job_AddFlags(j, AH_JOB_FLAGS_MULTIMSG);
    /* a multi message job must be single, too */
    AH_Job_AddFlags(j, AH_JOB_FLAGS_SINGLE);
    j->msgNode=msgNode;
    AH_Job_AddFlags(j, (GWEN_XMLNode_GetIntProperty(msgNode, "sign", 1)!=0)?(AH_JOB_FLAGS_NEEDSIGN | AH_JOB_FLAGS_SIGN):0);
    AH_Job_AddFlags(j, (GWEN_XMLNode_GetIntProperty(msgNode, "crypt", 1)!=0)?(AH_JOB_FLAGS_NEEDCRYPT | AH_JOB_FLAGS_CRYPT):0);
    AH_Job_AddFlags(j, (GWEN_XMLNode_GetIntProperty(msgNode, "needtan", 0)!=0)?AH_JOB_FLAGS_NEEDTAN:0);
    AH_Job_AddFlags(j, (GWEN_XMLNode_GetIntProperty(msgNode, "nosysid", 0)!=0)?(AH_JOB_FLAGS_NOSYSID | AH_JOB_FLAGS_SINGLE):0);
    AH_Job_AddFlags(j, (GWEN_XMLNode_GetIntProperty(msgNode, "signseqone", 0)!=0)?AH_JOB_FLAGS_SIGNSEQONE:0);
    AH_Job_AddFlags(j, (GWEN_XMLNode_GetIntProperty(msgNode, "noitan", 0)!=0)?AH_JOB_FLAGS_NOITAN:0);
  } /* if msgNode */
  else {
    DBG_INFO(AQHBCI_LOGDOMAIN, "Single message job");
    AH_Job_AddFlags(j, (GWEN_XMLNode_GetIntProperty(jobNode, "sign", 1)!=0)?(AH_JOB_FLAGS_NEEDSIGN | AH_JOB_FLAGS_SIGN):0);
    AH_Job_AddFlags(j, (GWEN_XMLNode_GetIntProperty(jobNode, "crypt", 1)!=0)?(AH_JOB_FLAGS_NEEDCRYPT | AH_JOB_FLAGS_CRYPT):0);
    AH_Job_AddFlags(j, (GWEN_XMLNode_GetIntProperty(jobNode, "needtan", 0)!=0)?AH_JOB_FLAGS_NEEDTAN:0);
    AH_Job_AddFlags(j, (GWEN_XMLNode_GetIntProperty(jobNode, "nosysid", 0)!=0)?(AH_JOB_FLAGS_NOSYSID | AH_JOB_FLAGS_SINGLE):0);
    AH_Job_AddFlags(j, (GWEN_XMLNode_GetIntProperty(jobNode, "signseqone", 0)!=0)?AH_JOB_FLAGS_SIGNSEQONE:0);
    AH_Job_AddFlags(j, (GWEN_XMLNode_GetIntProperty(jobNode, "noitan", 0)!=0)?AH_JOB_FLAGS_NOITAN:0);
  }

  if (AH_Job_GetFlags(j) & (AH_JOB_FLAGS_NEEDSIGN | AH_JOB_FLAGS_SIGN)) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "%s: Signature needed according to job description in our XML files", j->name);
    j->minSigs=1;
  }
  else {
    DBG_INFO(AQHBCI_LOGDOMAIN, "%s: No signature needed according to job description in XML our files", j->name);
    j->minSigs=0;
  }
}



void _jobReadFromBpdParamsNode(AH_JOB *j, GWEN_DB_NODE *jobBPD)
{
  GWEN_DB_AddGroupChildren(j->jobParams, jobBPD);
  /* sample some variables from BPD jobs */
  j->minSigs=GWEN_DB_GetIntValue(jobBPD, "minsigs", 0, 0);
  if (j->minSigs>0) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "%s: Signature needed according to BPD", j->name);
    AH_Job_AddFlags(j, AH_JOB_FLAGS_NEEDSIGN | AH_JOB_FLAGS_SIGN);
  }
  else {
    AH_Job_SubFlags(j, AH_JOB_FLAGS_NEEDSIGN | AH_JOB_FLAGS_SIGN);
    DBG_INFO(AQHBCI_LOGDOMAIN, "%s: No signature needed according to BPD", j->name);
  }

  j->secProfile=GWEN_DB_GetIntValue(jobBPD, "secProfile", 0, 1);
  j->secClass=GWEN_DB_GetIntValue(jobBPD, "securityClass", 0, 0);
  j->jobsPerMsg=GWEN_DB_GetIntValue(jobBPD, "jobspermsg", 0, 0);
}



void _jobReadFromUpdNode(AH_JOB *j, GWEN_DB_NODE *jobUPD)
{
  GWEN_DB_NODE *dgr;

  dgr=GWEN_DB_GetGroup(j->jobParams, GWEN_DB_FLAGS_OVERWRITE_GROUPS, "upd");
  assert(dgr);
  GWEN_DB_AddGroupChildren(dgr, jobUPD);

  j->minSigs=GWEN_DB_GetIntValue(jobUPD, "minsign", 0, 0);
  if (j->minSigs>0) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "%s: Signature needed according to UPD", j->name);
    AH_Job_AddFlags(j, AH_JOB_FLAGS_NEEDSIGN | AH_JOB_FLAGS_SIGN);
  }
  else {
    AH_Job_SubFlags(j, AH_JOB_FLAGS_NEEDSIGN | AH_JOB_FLAGS_SIGN);
    DBG_INFO(AQHBCI_LOGDOMAIN, "%s: No signature needed according to UPD", j->name);
  }
}



void _jobReadSepaDescriptors(AH_JOB *j)
{
  GWEN_DB_NODE *dbT;

  dbT=GWEN_DB_FindFirstGroup(j->jobParams, "SupportedSepaFormats");
  if (dbT) {
    GWEN_STRINGLIST *descriptors;
    unsigned int i;
    const char *s;

    descriptors=GWEN_StringList_new();
    while (dbT) {
      for (i=0; i<10; i++) {
        s=GWEN_DB_GetCharValue(dbT, "format", i, NULL);
        if (!(s && *s))
          break;
        GWEN_StringList_AppendString(descriptors, s, 0, 1);
      }
      dbT=GWEN_DB_FindNextGroup(dbT, "SupportedSepaFormats");
    }
    if (GWEN_StringList_Count(descriptors)>0)
      j->sepaDescriptors=descriptors;
    else
      GWEN_StringList_free(descriptors);
  }
}





