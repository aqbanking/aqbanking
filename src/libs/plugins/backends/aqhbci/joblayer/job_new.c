/***************************************************************************
    begin       : Mon Mar 01 2004
    copyright   : (C) 2019 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


/* file included by job.c */




AH_JOB *AH_Job_new(const char *name,
                   AB_PROVIDER *pro,
                   AB_USER *u,
                   AB_ACCOUNT *acc,
                   int jobVersion)
{
  AH_JOB *j;
  GWEN_XMLNODE *node;
  GWEN_XMLNODE *jobNode=0;
  GWEN_XMLNODE *msgNode;
  GWEN_XMLNODE *descrNode;
  const char *segCode=NULL;
  const char *paramName;
  const char *responseName;
  int needsBPD;
  int needTAN;
  int noSysId;
  int noItan;
  int signSeqOne;
  GWEN_DB_NODE *bpdgrp;
  const AH_BPD *bpd;
  GWEN_MSGENGINE *e;

  assert(name);
  assert(u);

  needTAN=0;
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

  /* get job descriptions */

  e=AH_User_GetMsgEngine(u);
  assert(e);

  bpd=AH_User_GetBpd(u);

  /* just to make sure the XMLNode is not freed before this job is */
  j->msgEngine=e;
  GWEN_MsgEngine_Attach(e);
  if (AH_User_GetHbciVersion(u)==0)
    GWEN_MsgEngine_SetProtocolVersion(e, 300);
  else
    GWEN_MsgEngine_SetProtocolVersion(e, AH_User_GetHbciVersion(u));

  GWEN_MsgEngine_SetMode(e, AH_CryptMode_toString(AH_User_GetCryptMode(u)));

  /* first select any version, we simply need to know the BPD job name */
  node=GWEN_MsgEngine_FindNodeByProperty(e, "JOB", "id", 0, name);
  if (!node) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "Job \"%s\" not supported by local XML files", name);
    AH_Job_free(j);
    return 0;
  }
  jobNode=node;

  j->jobParams=GWEN_DB_Group_new("jobParams");
  j->jobArguments=GWEN_DB_Group_new("jobArguments");
  j->jobResponses=GWEN_DB_Group_new("jobResponses");

  /* get some properties */
  needsBPD=(atoi(GWEN_XMLNode_GetProperty(node, "needbpd", "0"))!=0); /* TODO: use GetIntProperty */
  needTAN=(atoi(GWEN_XMLNode_GetProperty(node, "needtan", "0"))!=0);
  noSysId=(atoi(GWEN_XMLNode_GetProperty(node, "nosysid", "0"))!=0);
  signSeqOne=(atoi(GWEN_XMLNode_GetProperty(node, "signseqone", "0"))!=0);
  noItan=(atoi(GWEN_XMLNode_GetProperty(node, "noitan", "0"))!=0);
  paramName=GWEN_XMLNode_GetProperty(node, "params", "");
  responseName=GWEN_XMLNode_GetProperty(node, "response", "");
  free(j->responseName);
  if (responseName)
    j->responseName=strdup(responseName);
  else
    j->responseName=NULL;

  /* get and store segment code for later use in TAN jobs */
  segCode=GWEN_XMLNode_GetProperty(node, "code", "");
  free(j->code);
  if (segCode && *segCode)
    j->code=strdup(segCode);
  else
    j->code=NULL;

  if (bpd) {
    bpdgrp=AH_Bpd_GetBpdJobs(bpd, AH_User_GetHbciVersion(u));
    assert(bpdgrp);
  }
  else
    bpdgrp=0;

  if (paramName && *paramName) {
    GWEN_DB_NODE *jobBPD;
    GWEN_DB_NODE *jobPT;
    GWEN_DB_NODE *dbHighestVersion;
    int highestVersion;

    DBG_INFO(AQHBCI_LOGDOMAIN, "Job \"%s\" needs BPD job \"%s\"", name, paramName);

    if (!bpd) {
      DBG_ERROR(AQHBCI_LOGDOMAIN, "No BPD");
      AH_Job_free(j);
      return 0;
    }

    /* get BPD job */
    jobBPD=GWEN_DB_GetGroup(bpdgrp, GWEN_PATH_FLAGS_NAMEMUSTEXIST, paramName);
    if (jobBPD) {
      /* children are one group per version */
      jobBPD=GWEN_DB_GetFirstGroup(jobBPD);
    }

    jobPT=GWEN_DB_GetGroup(bpdgrp, GWEN_PATH_FLAGS_NAMEMUSTEXIST, segCode);
    if (jobPT) {
      /* sample flag NEEDTAN */
      needTAN=GWEN_DB_GetIntValue(jobPT, "needTan", 0, needTAN);
    }

    /* check for a job for which we have a BPD */
    node=NULL;
    dbHighestVersion=NULL;
    highestVersion=-1;

    if (jobVersion) {
      /* a job version has been selected from outside, look for
       * the BPD of that particular version */
      while (jobBPD) {
        int version;

        /* get version from BPD */
        version=atoi(GWEN_DB_GroupName(jobBPD));
        if (version==jobVersion) {
          /* now get the correct version of the JOB */
          DBG_INFO(AQHBCI_LOGDOMAIN, "Checking Job %s (%d)", name, version);
          node=GWEN_MsgEngine_FindNodeByProperty(e, "JOB", "id", version, name);
          if (node) {
            dbHighestVersion=jobBPD;
            highestVersion=version;
            jobNode=node;
          }
        }
        jobBPD=GWEN_DB_GetNextGroup(jobBPD);
      } /* while */
      jobBPD=dbHighestVersion;
    }
    else {
      while (jobBPD) {
        int version;

        /* get version from BPD */
        version=atoi(GWEN_DB_GroupName(jobBPD));
        if (version>highestVersion) {
          /* now get the correct version of the JOB */
          DBG_INFO(AQHBCI_LOGDOMAIN, "Checking Job %s (%d)", name, version);
          node=GWEN_MsgEngine_FindNodeByProperty(e, "JOB", "id", version, name);
	  if (node) {
	    dbHighestVersion=jobBPD;
	    highestVersion=version;
	    jobNode=node;
	  }
	}
	jobBPD=GWEN_DB_GetNextGroup(jobBPD);
      } /* while */
      jobBPD=dbHighestVersion;
    }

    if (!jobBPD) {
      if (needsBPD) {
        if (AH_User_GetCryptMode(u)!=AH_CryptMode_Pintan &&
	    strcasecmp(name, "JobTan")==0) {
          /* lower loglevel for JobTan in non-PINTAN mode because this is often confusing */
	  DBG_INFO(AQHBCI_LOGDOMAIN, "Job \"%s\" not supported by your bank", name);
        }
        else {
          /* no BPD when needed, error */
	  DBG_WARN(AQHBCI_LOGDOMAIN, "Job \"%s\" not supported by your bank", name);
        }
        AH_Job_free(j);
        return 0;
      }
    }
    else {
      DBG_INFO(AQHBCI_LOGDOMAIN, "Highest version for job \"%s\" is %d", name, highestVersion);
      GWEN_DB_AddGroupChildren(j->jobParams, jobBPD);
      /* sample some variables from BPD jobs */
      j->segmentVersion=highestVersion;
      j->minSigs=GWEN_DB_GetIntValue(jobBPD, "minsigs", 0, 0);
      j->secProfile=GWEN_DB_GetIntValue(jobBPD, "secProfile", 0, 1);
      j->secClass=GWEN_DB_GetIntValue(jobBPD, "securityClass", 0, 0);
      j->jobsPerMsg=GWEN_DB_GetIntValue(jobBPD, "jobspermsg", 0, 0);
    }
  } /* if paramName */

  /* get UPD jobs (if any) */
  if (acc) {
    GWEN_DB_NODE *updgroup;
    GWEN_DB_NODE *updnode=NULL;

    updgroup=AH_User_GetUpdForAccount(u, acc);
    if (updgroup) {
      const char *code;

      code=GWEN_XMLNode_GetProperty(jobNode, "code", 0);
      if (code) {
        DBG_NOTICE(AQHBCI_LOGDOMAIN, "Code is \"%s\"", code);
        updnode=GWEN_DB_GetFirstGroup(updgroup);
        while (updnode) {
	  if (strcasecmp(GWEN_DB_GetCharValue(updnode, "job", 0, ""), code)==0) {
	    break;
	  }
          updnode=GWEN_DB_GetNextGroup(updnode);
        } /* while */
      } /* if code given */
    } /* if updgroup for the given account found */
    if (updnode) {
      GWEN_DB_NODE *dgr;

      /* upd job found */
      dgr=GWEN_DB_GetGroup(j->jobParams, GWEN_DB_FLAGS_OVERWRITE_GROUPS, "upd");
      assert(dgr);
      GWEN_DB_AddGroupChildren(dgr, updnode);
    }
    else if (needsBPD) {
      DBG_INFO(AQHBCI_LOGDOMAIN, "Job \"%s\" not enabled for account \"%u\"",
               name, AB_Account_GetUniqueId(acc));
      AH_Job_free(j);
      return NULL;
    }
  } /* if accountId given */

  /* sample flags from XML file */
  if (atoi(GWEN_XMLNode_GetProperty(jobNode, "dlg", "0"))!=0) {
    j->flags|=AH_JOB_FLAGS_DLGJOB;
    j->flags|=AH_JOB_FLAGS_SINGLE;
  }
  if (atoi(GWEN_XMLNode_GetProperty(jobNode, "attachable", "0"))!=0)
    j->flags|=AH_JOB_FLAGS_ATTACHABLE;
  if (atoi(GWEN_XMLNode_GetProperty(jobNode, "single", "0"))!=0)
    j->flags|=AH_JOB_FLAGS_SINGLE;

  if (atoi(GWEN_XMLNode_GetProperty(jobNode, "ignoreAccounts", "0"))!=0)
    j->flags|=AH_JOB_FLAGS_IGNOREACCOUNTS;

  /* sample other flags */
  if (AH_User_GetCryptMode(u)==AH_CryptMode_Pintan) {
    /* always make jobs single when in PIN/TAN mode */
    j->flags|=AH_JOB_FLAGS_SINGLE;
  }
  if (needTAN) {
    j->flags|=AH_JOB_FLAGS_NEEDTAN;
    DBG_INFO(AQHBCI_LOGDOMAIN, "This job needs a TAN");
  }

  if (noSysId) {
    j->flags|=AH_JOB_FLAGS_NOSYSID;
    j->flags|=AH_JOB_FLAGS_SINGLE;
  }

  if (signSeqOne) {
    j->flags|=AH_JOB_FLAGS_SIGNSEQONE;
  }

  if (noItan) {
    j->flags|=AH_JOB_FLAGS_NOITAN;
  }


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
    j->flags|=(AH_JOB_FLAGS_MULTIMSG);
    /* a multi message job must be single, too */
    j->flags|=AH_JOB_FLAGS_SINGLE;
    j->msgNode=msgNode;
    if (atoi(GWEN_XMLNode_GetProperty(msgNode, "sign", "1"))!=0) {
      if (j->minSigs==0)
        j->minSigs=1;
      j->flags|=(AH_JOB_FLAGS_NEEDSIGN | AH_JOB_FLAGS_SIGN);
    }
    if (atoi(GWEN_XMLNode_GetProperty(msgNode, "crypt", "1"))!=0)
      j->flags|=(AH_JOB_FLAGS_NEEDCRYPT| AH_JOB_FLAGS_CRYPT);
    if (atoi(GWEN_XMLNode_GetProperty(msgNode, "nosysid", "0"))!=0)
      j->flags|=AH_JOB_FLAGS_NOSYSID;
    if (atoi(GWEN_XMLNode_GetProperty(msgNode, "signseqone", "0"))!=0)
      j->flags|=AH_JOB_FLAGS_SIGNSEQONE;
    if (atoi(GWEN_XMLNode_GetProperty(msgNode, "noitan", "0"))!=0) {
      j->flags|=AH_JOB_FLAGS_NOITAN;
    }
  } /* if msgNode */
  else {
    DBG_INFO(AQHBCI_LOGDOMAIN, "Single message job");
    if (atoi(GWEN_XMLNode_GetProperty(jobNode, "sign", "1"))!=0) {
      if (j->minSigs==0)
        j->minSigs=1;
      j->flags|=(AH_JOB_FLAGS_NEEDSIGN | AH_JOB_FLAGS_SIGN);
    }
    if (atoi(GWEN_XMLNode_GetProperty(jobNode, "crypt", "1"))!=0)
      j->flags|=(AH_JOB_FLAGS_NEEDCRYPT| AH_JOB_FLAGS_CRYPT);
  }

  j->flags|=AH_JOB_FLAGS_HASMOREMSGS;
  j->xmlNode=jobNode;

  j->segResults=AH_Result_List_new();
  j->msgResults=AH_Result_List_new();
  j->messages=AB_Message_List_new();

  /* check BPD for job specific SEPA descriptors */
  if (needsBPD) {
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

  AH_Job_Log(j, GWEN_LoggerLevel_Info, "HBCI-Job created");

  return j;
}


