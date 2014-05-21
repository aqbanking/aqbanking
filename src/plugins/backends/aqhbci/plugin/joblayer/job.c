/***************************************************************************
    begin       : Mon Mar 01 2004
    copyright   : (C) 2004-2011 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#ifdef HAVE_CONFIG_H
# include <config.h>
#endif


#include "job_p.h"
#include "aqhbci_l.h"
#include "hbci_l.h"
#include "user_l.h"
#include <aqhbci/provider.h>
#include <aqbanking/job_be.h>
#include <gwenhywfar/debug.h>
#include <gwenhywfar/misc.h>
#include <gwenhywfar/gui.h>
#include <gwenhywfar/text.h>
#include <gwenhywfar/stringlist.h>

#include <stdlib.h>
#include <assert.h>
#include <string.h>



GWEN_LIST_FUNCTIONS(AH_JOB, AH_Job);
GWEN_LIST2_FUNCTIONS(AH_JOB, AH_Job);
GWEN_INHERIT_FUNCTIONS(AH_JOB);



AH_JOB *AH_Job_new(const char *name,
		   AB_USER *u,
		   const char *accountId,
		   int jobVersion) {
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
  if (accountId)
    j->accountId=strdup(accountId);
  j->user=u;
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
    AH_Job_free(j);
    return 0;
  }
  jobNode=node;

  j->jobParams=GWEN_DB_Group_new("jobParams");
  j->jobArguments=GWEN_DB_Group_new("jobArguments");
  j->jobResponses=GWEN_DB_Group_new("jobResponses");

  /* get some properties */
  needsBPD=(atoi(GWEN_XMLNode_GetProperty(node, "needbpd", "0"))!=0);
  needTAN=(atoi(GWEN_XMLNode_GetProperty(node, "needtan", "0"))!=0);
  noSysId=(atoi(GWEN_XMLNode_GetProperty(node, "nosysid", "0"))!=0);
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
  if (segCode && *segCode) j->code=strdup(segCode);
  else j->code=NULL;

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

    DBG_INFO(AQHBCI_LOGDOMAIN, "Job \"%s\" needs BPD job \"%s\"",
             name, paramName);

    if (!bpd) {
      DBG_ERROR(AQHBCI_LOGDOMAIN,"No BPD");
      AH_Job_free(j);
      return 0;
    }

    /* get BPD job */
    jobBPD=GWEN_DB_GetGroup(bpdgrp,
                            GWEN_PATH_FLAGS_NAMEMUSTEXIST,
                            paramName);
    if (jobBPD) {
      /* children are one group per version */
      jobBPD=GWEN_DB_GetFirstGroup(jobBPD);
    }

    jobPT=GWEN_DB_GetGroup(bpdgrp,
                           GWEN_PATH_FLAGS_NAMEMUSTEXIST,
                           segCode);
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
      while(jobBPD) {
	int version;

	/* get version from BPD */
	version=atoi(GWEN_DB_GroupName(jobBPD));
	if (version==jobVersion) {
	  /* now get the correct version of the JOB */
	  DBG_INFO(AQHBCI_LOGDOMAIN, "Checking Job %s (%d)",
		   name, version);
	  node=GWEN_MsgEngine_FindNodeByProperty(e,
						 "JOB",
						 "id",
						 version,
						 name);
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
      while(jobBPD) {
	int version;

	/* get version from BPD */
	version=atoi(GWEN_DB_GroupName(jobBPD));
	if (version>highestVersion) {
	  /* now get the correct version of the JOB */
	  DBG_INFO(AQHBCI_LOGDOMAIN, "Checking Job %s (%d)",
		   name, version);
	  node=GWEN_MsgEngine_FindNodeByProperty(e,
						 "JOB",
						 "id",
						 version,
						 name);
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
	  /* lower loglevel for JobTan in non-PINTAN mode because this is
	   * often confusing */
	  DBG_INFO(AQHBCI_LOGDOMAIN,
		   "Job \"%s\" not supported by your bank",
		   name);
	}
	else {
	  /* no BPD when needed, error */
	  DBG_WARN(AQHBCI_LOGDOMAIN,
		    "Job \"%s\" not supported by your bank",
		    name);
	}
	AH_Job_free(j);
	return 0;
      }
    }
    else {
      DBG_INFO(AQHBCI_LOGDOMAIN, "Highest version is %d", highestVersion);
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
  if (accountId) {
    GWEN_DB_NODE *updgroup;
    GWEN_DB_NODE *updnode=NULL;

    updgroup=AH_User_GetUpd(u);
    assert(updgroup);
    updgroup=GWEN_DB_GetGroup(updgroup, GWEN_PATH_FLAGS_NAMEMUSTEXIST,
                              accountId);
    if (updgroup) {
      const char *code;

      code=GWEN_XMLNode_GetProperty(jobNode, "code", 0);
      if (code) {
        DBG_NOTICE(AQHBCI_LOGDOMAIN, "Code is \"%s\"", code);
        updnode=GWEN_DB_GetFirstGroup(updgroup);
        while(updnode) {
          if (strcasecmp(GWEN_DB_GetCharValue(updnode, "job", 0, ""),
                         code)==0) {
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
      DBG_INFO(AQHBCI_LOGDOMAIN,"Job \"%s\" not enabled for account \"%s\"",
               name, accountId);
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
    while(dn) {
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
      while(dbT) {
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

  AH_Job_Log(j, GWEN_LoggerLevel_Info,
             "HBCI-Job created");

  return j;
}



void AH_Job_free(AH_JOB *j) {
  if (j) {
    assert(j->usage);
    if (--(j->usage)==0) {
      AB_Value_free(j->challengeValue);
      GWEN_StringList_free(j->challengeParams);
      GWEN_StringList_free(j->log);
      GWEN_StringList_free(j->signers);
      GWEN_StringList_free(j->sepaDescriptors);
      free(j->responseName);
      free(j->code);
      free(j->name);
      free(j->accountId);
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

      GWEN_LIST_FINI(AH_JOB, j);
      GWEN_INHERIT_FINI(AH_JOB, j);
      GWEN_FREE_OBJECT(j);
    }
  }
}



int AH_Job_SampleBpdVersions(const char *name,
			     AB_USER *u,
			     GWEN_DB_NODE *dbResult) {
  GWEN_XMLNODE *node;
  const char *paramName;
  GWEN_DB_NODE *bpdgrp;
  const AH_BPD *bpd;
  GWEN_MSGENGINE *e;

  assert(name);
  assert(u);

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
      DBG_ERROR(AQHBCI_LOGDOMAIN,"No BPD");
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
    while(jobBPD) {
      int version;

      /* get version from BPD */
      version=atoi(GWEN_DB_GroupName(jobBPD));
      /* now get the correct version of the JOB */
      DBG_INFO(AQHBCI_LOGDOMAIN, "Checking Job %s (%d)", name, version);
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



int AH_Job_GetMaxVersionUpUntil(const char *name, AB_USER *u, int maxVersion) {
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
    while(dbT) {
      int v;

      v=atoi(GWEN_DB_GroupName(dbT));
      if (v>0 && v>m && v<=maxVersion)
        m=v;
      dbT=GWEN_DB_GetNextGroup(dbT);
    }
    GWEN_DB_Group_free(db);
    DBG_INFO(AQHBCI_LOGDOMAIN, "Max version of [%s] up until %d: %d",
	     name, maxVersion, m);
    return m;
  }
}



AB_MESSAGE_LIST *AH_Job_GetMessages(const AH_JOB *j) {
  assert(j);
  assert(j->usage);
  return j->messages;
}



int AH_Job_GetChallengeClass(const AH_JOB *j) {
  assert(j);
  assert(j->usage);
  return j->challengeClass;
}



int AH_Job_GetSegmentVersion(const AH_JOB *j) {
  assert(j);
  assert(j->usage);
  return j->segmentVersion;
}



void AH_Job_SetChallengeClass(AH_JOB *j, int i) {
  assert(j);
  assert(j->usage);
  j->challengeClass=i;
}



void AH_Job_Attach(AH_JOB *j) {
  assert(j);
  assert(j->usage);
  j->usage++;
}



int AH_Job_PrepareNextMessage(AH_JOB *j) {
  assert(j);
  assert(j->usage);

  if (j->nextMsgFn) {
    int rv;

    rv=j->nextMsgFn(j);
    if (rv==0) {
      /* callback flagged that no message follows */
      DBG_INFO(AQHBCI_LOGDOMAIN, "Job says: No more messages");
      j->flags&=~AH_JOB_FLAGS_HASMOREMSGS;
      return 0;
    }
    else if (rv!=1) {
      DBG_INFO(AQHBCI_LOGDOMAIN, "Job says: Error");
      j->flags&=~AH_JOB_FLAGS_HASMOREMSGS;
      return rv;
    }
  }

  if (j->status==AH_JobStatusUnknown ||
      j->status==AH_JobStatusError) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "At least one message had errors, aborting job");
    j->flags&=~AH_JOB_FLAGS_HASMOREMSGS;
    return 0;
  }

  if (j->status==AH_JobStatusToDo) {
      DBG_NOTICE(AQHBCI_LOGDOMAIN,
                 "Hmm, job has never been sent, so we do nothing here");
    j->flags&=~AH_JOB_FLAGS_HASMOREMSGS;
    return 0;
  }

  if (j->flags & AH_JOB_FLAGS_HASATTACHPOINT) {
      DBG_NOTICE(AQHBCI_LOGDOMAIN,
                 "Job has an attachpoint, so yes, we need more messages");
    j->flags|=AH_JOB_FLAGS_HASMOREMSGS;
    AH_Job_Log(j, GWEN_LoggerLevel_Debug,
               "Job has an attachpoint");
    return 1;
  }

  if (!(j->flags & AH_JOB_FLAGS_MULTIMSG)) {
    DBG_NOTICE(AQHBCI_LOGDOMAIN, "Not a Multi-message job, so we don't need more messages");
    j->flags&=~AH_JOB_FLAGS_HASMOREMSGS;
    return 0;
  }

  assert(j->msgNode);
  j->msgNode=GWEN_XMLNode_FindNextTag(j->msgNode, "MESSAGE", 0, 0);
  if (j->msgNode) {
    /* there is another message, so set flags accordingly */
    DBG_NOTICE(AQHBCI_LOGDOMAIN, "Multi-message job, still more messages");
    AH_Job_Log(j, GWEN_LoggerLevel_Debug,
               "Job has more messages");

    /* sample some flags for the next message */
    if (atoi(GWEN_XMLNode_GetProperty(j->msgNode, "sign", "1"))!=0) {
      if (j->minSigs==0)
        j->minSigs=1;
      j->flags|=(AH_JOB_FLAGS_NEEDSIGN | AH_JOB_FLAGS_SIGN);
    }
    else {
      j->flags&=~(AH_JOB_FLAGS_NEEDSIGN | AH_JOB_FLAGS_SIGN);
    }
    if (atoi(GWEN_XMLNode_GetProperty(j->msgNode, "crypt", "1"))!=0)
      j->flags|=(AH_JOB_FLAGS_NEEDCRYPT| AH_JOB_FLAGS_CRYPT);
    else
      j->flags&=~(AH_JOB_FLAGS_NEEDCRYPT| AH_JOB_FLAGS_CRYPT);

    if (atoi(GWEN_XMLNode_GetProperty(j->msgNode, "nosysid", "0"))!=0)
      j->flags|=AH_JOB_FLAGS_NOSYSID;
    else
      j->flags&=~AH_JOB_FLAGS_NOSYSID;

    if (atoi(GWEN_XMLNode_GetProperty(j->msgNode, "noitan", "0"))!=0) {
      j->flags|=AH_JOB_FLAGS_NOITAN;
    }
    else
      j->flags&=~AH_JOB_FLAGS_NOITAN;

    if (atoi(GWEN_XMLNode_GetProperty(j->msgNode, "ignerrors", "0"))!=0)
      j->flags|=AH_JOB_FLAGS_IGNORE_ERROR;
    else
      j->flags&=~AH_JOB_FLAGS_IGNORE_ERROR;

    j->flags|=AH_JOB_FLAGS_HASMOREMSGS;
    return 1;
  }
  else {
    DBG_NOTICE(AQHBCI_LOGDOMAIN, "Job \"%s\" is finished", j->name);
    AH_Job_Log(j, GWEN_LoggerLevel_Debug,
               "Job has no more messages");
    j->flags&=~AH_JOB_FLAGS_HASMOREMSGS;
    return 0;
  }
}



uint32_t AH_Job_GetId(const AH_JOB *j){
  assert(j);
  assert(j->usage);
  return j->id;
}



void AH_Job_SetId(AH_JOB *j, uint32_t i){
  assert(j);
  assert(j->usage);
  j->id=i;
}



const char *AH_Job_GetName(const AH_JOB *j) {
  assert(j);
  assert(j->usage);
  return j->name;
}



const char *AH_Job_GetCode(const AH_JOB *j) {
  assert(j);
  assert(j->usage);
  return j->code;
}



const char *AH_Job_GetResponseName(const AH_JOB *j) {
  assert(j);
  assert(j->usage);
  return j->responseName;
}



int AH_Job_GetMinSignatures(const AH_JOB *j){
  assert(j);
  assert(j->usage);
  return j->minSigs;
}



int AH_Job_GetSecurityProfile(const AH_JOB *j){
  assert(j);
  assert(j->usage);
  return j->secProfile;
}



int AH_Job_GetSecurityClass(const AH_JOB *j) {
  assert(j);
  assert(j->usage);
  return j->secClass;
}



int AH_Job_GetJobsPerMsg(const AH_JOB *j){
  assert(j);
  assert(j->usage);
  return j->jobsPerMsg;
}



uint32_t AH_Job_GetFlags(const AH_JOB *j) {
  assert(j);
  assert(j->usage);
  return j->flags;
}



void AH_Job_SetFlags(AH_JOB *j, uint32_t f) {
  assert(j);
  assert(j->usage);
  DBG_INFO(AQHBCI_LOGDOMAIN, "Changing flags of job \"%s\" from %08x to %08x",
           j->name, j->flags, f);
  j->flags=f;
}



void AH_Job_AddFlags(AH_JOB *j, uint32_t f){
  assert(j);
  assert(j->usage);
  DBG_INFO(AQHBCI_LOGDOMAIN,
           "Changing flags of job \"%s\" from %08x to %08x",
           j->name, j->flags, j->flags|f);
  j->flags|=f;
}



void AH_Job_SubFlags(AH_JOB *j, uint32_t f){
  assert(j);
  assert(j->usage);
  DBG_INFO(AQHBCI_LOGDOMAIN,
           "Changing flags of job \"%s\" from %08x to %08x",
           j->name, j->flags, j->flags&~f);
  j->flags&=~f;
}



GWEN_DB_NODE *AH_Job_GetParams(const AH_JOB *j){
  assert(j);
  assert(j->usage);
  return j->jobParams;
}



GWEN_DB_NODE *AH_Job_GetArguments(const AH_JOB *j){
  assert(j);
  assert(j->usage);
  return j->jobArguments;
}



GWEN_DB_NODE *AH_Job_GetResponses(const AH_JOB *j){
  assert(j);
  assert(j->usage);
  return j->jobResponses;
}



uint32_t AH_Job_GetFirstSegment(const AH_JOB *j){
  assert(j);
  assert(j->usage);
  return j->firstSegment;
}



void AH_Job_SetFirstSegment(AH_JOB *j, uint32_t i){
  assert(j);
  assert(j->usage);
  j->firstSegment=i;
}



uint32_t AH_Job_GetLastSegment(const AH_JOB *j){
  assert(j);
  assert(j->usage);
  return j->lastSegment;
}



void AH_Job_SetLastSegment(AH_JOB *j, uint32_t i){
  assert(j);
  assert(j->usage);
  j->lastSegment=i;
}



int AH_Job_HasSegment(const AH_JOB *j, int seg){
  assert(j);
  assert(j->usage);
  DBG_INFO(AQHBCI_LOGDOMAIN, "Job \"%s\" checked for %d: first=%d, last=%d",
           j->name,seg,  j->firstSegment, j->lastSegment);
  return (seg<=j->lastSegment && seg>=j->firstSegment);
}



void AH_Job_AddResponse(AH_JOB *j, GWEN_DB_NODE *db){
  assert(j);
  assert(j->usage);
  GWEN_DB_AddGroup(j->jobResponses, db);
}



AH_JOB_STATUS AH_Job_GetStatus(const AH_JOB *j){
  assert(j);
  assert(j->usage);
  return j->status;
}



void AH_Job_SetStatus(AH_JOB *j, AH_JOB_STATUS st){
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

    AH_Job_Log(j, GWEN_LoggerLevel_Info,
               GWEN_Buffer_GetStart(lbuf));
    GWEN_Buffer_free(lbuf);
    j->status=st;
  }
}



void AH_Job_AddSigner(AH_JOB *j, const char *s){
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
  }
  GWEN_Buffer_free(lbuf);
  j->flags|=AH_JOB_FLAGS_SIGN;
}



AB_USER *AH_Job_GetUser(const AH_JOB *j){
  assert(j);
  assert(j->usage);
  return j->user;
}



const GWEN_STRINGLIST *AH_Job_GetSigners(const AH_JOB *j){
  assert(j);
  assert(j->usage);
  return j->signers;
}



GWEN_XMLNODE *AH_Job_GetXmlNode(const AH_JOB *j){
  assert(j);
  assert(j->usage);
  if (j->flags & AH_JOB_FLAGS_MULTIMSG) {
    DBG_INFO(AQHBCI_LOGDOMAIN,
             "Multi message node, returning current message node");
    return j->msgNode;
  }
  return j->xmlNode;
}



unsigned int AH_Job_GetMsgNum(const AH_JOB *j){
  assert(j);
  assert(j->usage);
  return j->msgNum;
}



const char *AH_Job_GetDialogId(const AH_JOB *j){
  assert(j);
  assert(j->usage);
  return j->dialogId;
}



void AH_Job_SetMsgNum(AH_JOB *j, unsigned int i){
  assert(j);
  assert(j->usage);
  j->msgNum=i;
}



void AH_Job_SetDialogId(AH_JOB *j, const char *s){
  assert(j);
  assert(j->usage);
  assert(s);

  free(j->dialogId);
  j->dialogId=strdup(s);
}



const char *AH_Job_StatusName(AH_JOB_STATUS st) {
  switch(st) {
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



const char *AH_Job_GetAccountId(const AH_JOB *j) {
  assert(j);
  assert(j->usage);
  return j->accountId;
}



int AH_Job_Process(AH_JOB *j, AB_IMEXPORTER_CONTEXT *ctx){

  assert(j);
  assert(j->usage);

  AH_Job_SampleResults(j);

  if (j->processFn)
    return j->processFn(j, ctx);
  else {
    DBG_INFO(AQHBCI_LOGDOMAIN, "No processFn set");
    return AH_Job_DefaultProcessHandler(j);
  }
}



int AH_Job_Commit(AH_JOB *j, int doLock){
  assert(j);
  assert(j->usage);
  if (j->commitFn)
    return j->commitFn(j, doLock);
  else {
    DBG_DEBUG(AQHBCI_LOGDOMAIN, "No commitFn set");
    return AH_Job_DefaultCommitHandler(j, doLock);
  }
}



int AH_Job_Exchange(AH_JOB *j, AB_JOB *bj,
		    AH_JOB_EXCHANGE_MODE m,
		    AB_IMEXPORTER_CONTEXT *ctx){
  GWEN_DB_NODE *db;

  assert(j);
  assert(j->usage);

  DBG_INFO(AQHBCI_LOGDOMAIN, "Exchanging %d (%s)", m, j->name);

  db=AB_Job_GetProviderData(bj, AH_HBCI_GetProvider(AH_Job_GetHbci(j)));
  assert(db);

  switch(m) {
  case AH_Job_ExchangeModeParams: {
    AB_USER *u;

    u=AH_Job_GetUser(j);
    assert(u);
    GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS,
                         "customerId",
                         AB_User_GetCustomerId(u));
    GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS,
                         "bankId",
                         AB_User_GetBankCode(u));
    break;
  }
  case AH_Job_ExchangeModeArgs:
    /* no generic action here */
    break;
  case AH_Job_ExchangeModeResults:
    if (GWEN_DB_GetCharValue(db, "msgref/dialogId", 0, 0)==0) {
      const char *s;
      GWEN_DB_NODE *dbT;

      /* don't overwrite existing msgref */

      dbT=GWEN_DB_GetGroup(db, GWEN_DB_FLAGS_OVERWRITE_GROUPS, "msgref");
      assert(dbT);
      s=AH_Job_GetDialogId(j);
      if (s)
        GWEN_DB_SetCharValue(dbT, GWEN_DB_FLAGS_OVERWRITE_VARS,
                             "dialogId", s);
      GWEN_DB_SetIntValue(dbT, GWEN_DB_FLAGS_OVERWRITE_VARS,
                          "msgnum", AH_Job_GetMsgNum(j));
      GWEN_DB_SetIntValue(dbT, GWEN_DB_FLAGS_OVERWRITE_VARS,
                          "firstseg",
                          AH_Job_GetFirstSegment(j));
      GWEN_DB_SetIntValue(dbT, GWEN_DB_FLAGS_OVERWRITE_VARS,
                          "lastseg",
                          AH_Job_GetLastSegment(j));
    }
    break;

  default:
    DBG_ERROR(AQHBCI_LOGDOMAIN, "Unknown exchange mode %d", m);
    return GWEN_ERROR_NOT_SUPPORTED;
  } /* switch */

  if (j->exchangeFn)
    return j->exchangeFn(j, bj, m, ctx);
  else {
    DBG_INFO(AQHBCI_LOGDOMAIN, "No exchangeFn set");
    return GWEN_ERROR_NOT_SUPPORTED;
  }
}



int AH_Job_Prepare(AH_JOB *j){
  assert(j);
  assert(j->usage);
  if (j->prepareFn)
    return j->prepareFn(j);
  else {
    DBG_DEBUG(AQHBCI_LOGDOMAIN, "No prepareFn set");
    return GWEN_ERROR_NOT_SUPPORTED;
  }
}



int AH_Job_AddChallengeParams(AH_JOB *j, int hkTanVer, GWEN_DB_NODE *dbMethod) {
  assert(j);
  assert(j->usage);
  if (j->addChallengeParamsFn)
    return j->addChallengeParamsFn(j, hkTanVer, dbMethod);
  else {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "No addChallengeParamsFn set");
    return GWEN_ERROR_NOT_SUPPORTED;
  }
}



void AH_Job_SetProcessFn(AH_JOB *j, AH_JOB_PROCESS_FN f){
  assert(j);
  assert(j->usage);
  j->processFn=f;
}



void AH_Job_SetCommitFn(AH_JOB *j, AH_JOB_COMMIT_FN f){
  assert(j);
  assert(j->usage);
  j->commitFn=f;
}



void AH_Job_SetExchangeFn(AH_JOB *j, AH_JOB_EXCHANGE_FN f){
  assert(j);
  assert(j->usage);
  j->exchangeFn=f;
}



void AH_Job_SetNextMsgFn(AH_JOB *j, AH_JOB_NEXTMSG_FN f){
  assert(j);
  assert(j->usage);
  j->nextMsgFn=f;
}



void AH_Job_SetPrepareFn(AH_JOB *j, AH_JOB_PREPARE_FN f) {
  assert(j);
  assert(j->usage);
  j->prepareFn=f;
}



void AH_Job_SetAddChallengeParamsFn(AH_JOB *j, AH_JOB_ADDCHALLENGEPARAMS_FN f) {
  assert(j);
  assert(j->usage);
  j->addChallengeParamsFn=f;
}



int AH_Job_HasWarnings(const AH_JOB *j){
  assert(j);
  assert(j->usage);
  return (j->flags & AH_JOB_FLAGS_HASWARNINGS);
}



int AH_Job_HasErrors(const AH_JOB *j){
  assert(j);
  assert(j->usage);
  return
    (j->status==AH_JobStatusError) ||
    (j->flags & AH_JOB_FLAGS_HASERRORS);
}



void AH_Job_SampleResults(AH_JOB *j) {
  GWEN_DB_NODE *dbCurr;

  assert(j);
  assert(j->usage);

  dbCurr=GWEN_DB_GetFirstGroup(j->jobResponses);
  while(dbCurr) {
    GWEN_DB_NODE *dbResults;

    dbResults=GWEN_DB_GetGroup(dbCurr, GWEN_PATH_FLAGS_NAMEMUSTEXIST,
                              "data/SegResult");
    if (dbResults) {
      GWEN_DB_NODE *dbRes;

      dbRes=GWEN_DB_GetFirstGroup(dbResults);
      while(dbRes) {
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
            j->flags|=AH_JOB_FLAGS_HASERRORS;
          else if (code>=3000 && code<4000)
            j->flags|=AH_JOB_FLAGS_HASWARNINGS;
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
        while(dbRes) {
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
              j->flags|=AH_JOB_FLAGS_HASERRORS;
            }
            else if (code>=3000 && code<4000)
              j->flags|=AH_JOB_FLAGS_HASWARNINGS;
          } /* if result */
          dbRes=GWEN_DB_GetNextGroup(dbRes);
        } /* while */
      } /* if msgResult */
    }
    dbCurr=GWEN_DB_GetNextGroup(dbCurr);
  } /* while */

}



const char *AH_Job_GetDescription(const AH_JOB *j){
  assert(j);
  assert(j->usage);
  if (j->description)
    return j->description;
  return j->name;
}



void AH_Job_Dump(const AH_JOB *j, FILE *f, unsigned int insert) {
  uint32_t k;

  for (k=0; k<insert; k++)
    fprintf(f, " ");
  fprintf(f, "Job:\n");

  for (k=0; k<insert; k++)
    fprintf(f, " ");
  fprintf(f, "Name  : %s\n", j->name);

  for (k=0; k<insert; k++)
    fprintf(f, " ");
  fprintf(f, "Status: %s (%d)\n", AH_Job_StatusName(j->status),j->status);

  for (k=0; k<insert; k++)
    fprintf(f, " ");
  fprintf(f, "Msgnum: %d\n", j->msgNum);

  for (k=0; k<insert; k++)
    fprintf(f, " ");
  fprintf(f, "DialogId: %s\n", j->dialogId);

  for (k=0; k<insert; k++)
    fprintf(f, " ");
  fprintf(f, "Owner   : %s\n", AB_User_GetCustomerId(j->user));
}



int AH_Job__CommitSystemData(AH_JOB *j, int doLock) {
  GWEN_DB_NODE *dbCurr;
  AB_USER *u;
  AB_BANKING *ab;
  AH_HBCI *h;
  const char *p;
  int i;
  GWEN_MSGENGINE *e;
  int bpdDeleted;

  DBG_NOTICE(AQHBCI_LOGDOMAIN, "Committing data");
  assert(j);
  assert(j->usage);

  bpdDeleted=0;

  u=j->user;
  assert(u);
  h=AH_Job_GetHbci(j);
  assert(h);
  ab=AH_Job_GetBankingApi(j);
  assert(ab);
  e=AH_User_GetMsgEngine(j->user);
  assert(e);

  dbCurr=GWEN_DB_GetFirstGroup(j->jobResponses);
  while(dbCurr) {
    GWEN_DB_NODE *dbRd;

    dbRd=GWEN_DB_GetGroup(dbCurr, GWEN_PATH_FLAGS_NAMEMUSTEXIST, "data");
    if (dbRd) {
      dbRd=GWEN_DB_GetFirstGroup(dbRd);
    }
    if (dbRd) {
      DBG_NOTICE(AQHBCI_LOGDOMAIN,
                 "Checking group \"%s\"", GWEN_DB_GroupName(dbRd));
      if (strcasecmp(GWEN_DB_GroupName(dbRd), "bpd")==0){
        AH_BPD *bpd;
        GWEN_DB_NODE *n;

        DBG_NOTICE(AQHBCI_LOGDOMAIN, "Found BPD");
        bpd=AH_User_GetBpd(j->user);

        AH_Bpd_SetBpdVersion(bpd,
                             GWEN_DB_GetIntValue(dbRd,
                                                 "version",
                                                 0, 0));

        /* read bank name */
        p=GWEN_DB_GetCharValue(dbRd, "name", 0, 0);
        if (p)
          AH_Bpd_SetBankName(bpd, p);
        AH_Bpd_SetJobTypesPerMsg(bpd,
                                 GWEN_DB_GetIntValue(dbRd,
                                                     "jobtypespermsg",
                                                     0, 0));
        AH_Bpd_SetMaxMsgSize(bpd,
                             GWEN_DB_GetIntValue(dbRd,
                                                 "maxmsgsize",
                                                 0, 0));
        /* read languages */
        n=GWEN_DB_GetGroup(dbRd,
                           GWEN_PATH_FLAGS_NAMEMUSTEXIST,
                           "languages");
        if (n) {
          AH_Bpd_ClearLanguages(bpd);
          for (i=0;;i++) {
            int k;

            k=GWEN_DB_GetIntValue(n, "language", i, 0);
            if (k) {
              if (AH_Bpd_AddLanguage(bpd, k)) {
                DBG_ERROR(AQHBCI_LOGDOMAIN, "Too many languages (%d)", i);
                break;
              }
            }
            else
              break;
          } /* for */
        } /* if languages */

        /* read supported version */
        n=GWEN_DB_GetGroup(dbRd,
                           GWEN_PATH_FLAGS_NAMEMUSTEXIST,
                           "versions");
        if (n) {
          AH_Bpd_ClearHbciVersions(bpd);
          for (i=0;;i++) {
            int k;

            k=GWEN_DB_GetIntValue(n, "version", i, 0);
            if (k) {
              if (AH_Bpd_AddHbciVersion(bpd, k)) {
                DBG_ERROR(AQHBCI_LOGDOMAIN, "Too many versions (%d)", i);
                break;
              }
            }
            else
              break;
          } /* for */
        } /* if versions */
        /* FIXME: remove this AH_Bank_SetBpd(b, AH_Bpd_dup(bpd)); */
	if (!bpdDeleted) {
	  AH_Bpd_ClearBpdJobs(bpd);
	  AH_Bpd_ClearAddr(bpd);
          bpdDeleted=1;
	}
      } /* if BPD found */

      else if (strcasecmp(GWEN_DB_GroupName(dbRd), "ComData")==0){
	/* communication parameters */
	GWEN_DB_NODE *currService;
	AH_BPD *bpd;

	DBG_INFO(AQHBCI_LOGDOMAIN, "Found communication infos");
        bpd=AH_User_GetBpd(j->user);
	assert(bpd);

	if (!bpdDeleted) {
	  AH_Bpd_ClearBpdJobs(bpd);
          AH_Bpd_ClearAddr(bpd);
	  bpdDeleted=1;
	}

	currService=GWEN_DB_FindFirstGroup(dbRd, "service");
	while(currService) {
	  AH_BPD_ADDR *ba;

	  ba=AH_BpdAddr_FromDb(currService);
	  if (ba) {
            DBG_INFO(AQHBCI_LOGDOMAIN, "Adding service");
	    AH_Bpd_AddAddr(bpd, ba);
	  }
	  currService=GWEN_DB_FindNextGroup(currService, "service");
	}

      } /* if ComData found */

      else if (strcasecmp(GWEN_DB_GroupName(dbRd), "PinTanBPD")==0){
        /* special extension of BPD for PIN/TAN mode */
        GWEN_DB_NODE *bn;
        GWEN_DB_NODE *currJob;
        AH_BPD *bpd;

        bpd=AH_User_GetBpd(j->user);
        assert(bpd);
	if (!bpdDeleted) {
	  AH_Bpd_ClearBpdJobs(bpd);
          AH_Bpd_ClearAddr(bpd);
	  bpdDeleted=1;
	}
        bn=AH_Bpd_GetBpdJobs(bpd, GWEN_MsgEngine_GetProtocolVersion(e));
        assert(bn);

        currJob=GWEN_DB_FindFirstGroup(dbRd, "job");
        while(currJob) {
          const char *jobName;
          int needTAN;
          GWEN_DB_NODE *dbJob;

          jobName=GWEN_DB_GetCharValue(currJob, "job", 0, 0);
          assert(jobName);
          dbJob=GWEN_DB_GetGroup(bn, GWEN_DB_FLAGS_DEFAULT,
                                 jobName);
          assert(dbJob);
          needTAN=strcasecmp(GWEN_DB_GetCharValue(currJob,
                                                  "needTan",
                                                  0,
                                                  "N"),
                             "J")==0;
          GWEN_DB_SetIntValue(dbJob, GWEN_DB_FLAGS_OVERWRITE_VARS,
                              "needTan", needTAN);
          currJob=GWEN_DB_FindNextGroup(currJob, "job");
        } /* while */
      } /* if PIN/TAN extension found */

      else if (strcasecmp(GWEN_DB_GroupName(dbRd), "SegResult")==0){
        GWEN_DB_NODE *dbRes;

        dbRes=GWEN_DB_GetFirstGroup(dbRd);
        while(dbRes) {
          if (strcasecmp(GWEN_DB_GroupName(dbRes), "result")==0) {
            int code;
//            const char *text;
  
            code=GWEN_DB_GetIntValue(dbRes, "resultcode", 0, 0);
//            text=GWEN_DB_GetCharValue(dbRes, "text", 0, 0);
            if (code==3920) {
              int i;

              AH_User_ClearTanMethodList(u);
              for (i=0; ; i++) {
                int j;

                j=GWEN_DB_GetIntValue(dbRes, "param", i, 0);
		if (j==0)
		  break;
		DBG_NOTICE(AQHBCI_LOGDOMAIN, "Adding allowed TAN method %d", j);
		AH_User_AddTanMethod(u, j);
	      } /* for */
	      if (i==0) {
                /* add single step if empty list */
		DBG_INFO(AQHBCI_LOGDOMAIN, "No allowed TAN method reported, assuming 999");
		AH_User_AddTanMethod(u, 999);
	      }
	    }
          } /* if result */
          dbRes=GWEN_DB_GetNextGroup(dbRes);
        } /* while */
      }
      else if (strcasecmp(GWEN_DB_GroupName(dbRd), "GetKeyResponse")==0){
	/* TODO: Read the key received and ask the user to accept it */
      }

      else if (strcasecmp(GWEN_DB_GroupName(dbRd), "SecurityMethods")==0){
        GWEN_DB_NODE *dbT;

        dbT=GWEN_DB_FindFirstGroup(dbRd, "SecProfile");
	while(dbT) {
	  const char *code;
          int version;
  
	  code=GWEN_DB_GetCharValue(dbT, "code", 0, NULL);
	  version=GWEN_DB_GetIntValue(dbT, "version", 0, -1);
	  if (code && (version>0)) {
	    DBG_ERROR(AQHBCI_LOGDOMAIN,
		      "Bank supports mode %s %d",
		      code, version);
	    /* TODO: store possible modes */
	  }
	  dbT=GWEN_DB_FindNextGroup(dbT, "SecProfile");
	} /* while */
      }

      else {
        GWEN_XMLNODE *bpdn;
        int segver;
        /* check for BPD job */

        DBG_INFO(AQHBCI_LOGDOMAIN, "Checking whether \"%s\" is a BPD job",
                 GWEN_DB_GroupName(dbRd));
        segver=GWEN_DB_GetIntValue(dbRd, "head/version", 0, 0);
        /* get segment description (first try id, then code) */
        bpdn=GWEN_MsgEngine_FindNodeByProperty(e,
                                               "SEG",
                                               "id",
                                               segver,
                                               GWEN_DB_GroupName(dbRd));
        if (!bpdn)
          bpdn=GWEN_MsgEngine_FindNodeByProperty(e,
                                                 "SEG",
                                                 "code",
                                                 segver,
                                                 GWEN_DB_GroupName(dbRd));
        if (bpdn) {
          DBG_DEBUG(AQHBCI_LOGDOMAIN, "Found a candidate");
          if (atoi(GWEN_XMLNode_GetProperty(bpdn, "isbpdjob", "0"))) {
            /* segment contains a BPD job, move contents */
            GWEN_DB_NODE *bn;
            AH_BPD *bpd;
            char numbuffer[32];

            DBG_NOTICE(AQHBCI_LOGDOMAIN, "Found BPD job \"%s\"",
                       GWEN_DB_GroupName(dbRd));
            bpd=AH_User_GetBpd(j->user);
            assert(bpd);
	    if (!bpdDeleted) {
	      AH_Bpd_ClearBpdJobs(bpd);
	      AH_Bpd_ClearAddr(bpd);
	      bpdDeleted=1;
	    }
            bn=AH_Bpd_GetBpdJobs(bpd, GWEN_MsgEngine_GetProtocolVersion(e));
            assert(bn);
            bn=GWEN_DB_GetGroup(bn, GWEN_DB_FLAGS_DEFAULT,
                                GWEN_DB_GroupName(dbRd));
            assert(bn);

            if (GWEN_Text_NumToString(segver,
                                      numbuffer,
                                      sizeof(numbuffer)-1,
                                      0)<1) {
              DBG_NOTICE(AQHBCI_LOGDOMAIN, "Buffer too small");
              abort();
            }
            bn=GWEN_DB_GetGroup(bn, GWEN_DB_FLAGS_OVERWRITE_GROUPS,
                                numbuffer);
            assert(bn);

            GWEN_DB_AddGroupChildren(bn, dbRd);
            /* remove "head" and "segment" group */
            GWEN_DB_DeleteGroup(bn, "head");
	    GWEN_DB_DeleteGroup(bn, "segment");
	    DBG_INFO(AQHBCI_LOGDOMAIN, "Added BPD Job %s:%d",
                     GWEN_DB_GroupName(dbRd), segver);
          } /* if isbpdjob */
          else {
	    DBG_INFO(AQHBCI_LOGDOMAIN,
		     "Segment \"%s\" is known but not as a BPD job",
		     GWEN_DB_GroupName(dbRd));
	  }
        } /* if segment found */
        else {
          DBG_WARN(AQHBCI_LOGDOMAIN, "Did not find segment \"%s\" (%d) ignoring",
                   GWEN_DB_GroupName(dbRd), segver);
        }
      }

      if (strcasecmp(GWEN_DB_GroupName(dbRd), "UserData")==0){
        /* UserData found */
        DBG_NOTICE(AQHBCI_LOGDOMAIN, "Found UserData");
        AH_User_SetUpdVersion(j->user,
			      GWEN_DB_GetIntValue(dbRd,
						  "version",
						  0, 0));
      }
      else if (strcasecmp(GWEN_DB_GroupName(dbRd), "AccountData")==0 ||
	       strcasecmp(GWEN_DB_GroupName(dbRd), "AccountData2")==0){
	const char *accountId;
        const char *accountSuffix;
        const char *userName;
        const char *accountName;
        const char *bankCode;
	const char *custId;
        const char *iban;
        AB_ACCOUNT *acc;
        GWEN_DB_NODE *gr;
        AH_BPD *bpd;
        GWEN_DB_NODE *dbUpd;
	int accCreated;
	int mayModifyAcc=1;
        int rv;

        DBG_INFO(AQHBCI_LOGDOMAIN, "Found AccountData");
        AH_Job_Log(j, GWEN_LoggerLevel_Info,
                   "Job contains account data");

        /* account data found */
	accountId=GWEN_DB_GetCharValue(dbRd, "accountId", 0, 0);
	if (accountId==NULL)
	  accountId=I18N("AH_JOB|-- no account id --");
	accountSuffix=GWEN_DB_GetCharValue(dbRd, "accountSubId", 0, 0);
        accountName=GWEN_DB_GetCharValue(dbRd, "account/name", 0, 0);
        userName=GWEN_DB_GetCharValue(dbRd, "name1", 0, 0);
	iban=GWEN_DB_GetCharValue(dbRd, "iban", 0, 0);
	bankCode=GWEN_DB_GetCharValue(dbRd, "bankCode", 0, 0);
	if (bankCode==NULL)
	  bankCode=I18N("AH_JOB|-- no bank code --");
        assert(bankCode);
        custId=GWEN_DB_GetCharValue(dbRd, "customer", 0, 0);
        assert(custId);
	if (1) {
          GWEN_BUFFER *mbuf;

          mbuf=GWEN_Buffer_new(0, 128, 0, 1);
          GWEN_Buffer_AppendString(mbuf, I18N("Received account:"));
	  GWEN_Buffer_AppendString(mbuf, " ");
          GWEN_Buffer_AppendString(mbuf, bankCode);
          GWEN_Buffer_AppendString(mbuf, " / ");
          if (accountName)
            GWEN_Buffer_AppendString(mbuf, accountName);
          else
            GWEN_Buffer_AppendString(mbuf, accountId);
	  if (accountSuffix) {
	    GWEN_Buffer_AppendString(mbuf, "(");
	    GWEN_Buffer_AppendString(mbuf, accountSuffix);
	    GWEN_Buffer_AppendString(mbuf, ")");
	  }
	  GWEN_Gui_ProgressLog(0,
			       GWEN_LoggerLevel_Notice,
			       GWEN_Buffer_GetStart(mbuf));
	  GWEN_Buffer_free(mbuf);
        }

        acc=AB_Banking_FindAccount(ab, AH_PROVIDER_NAME,
                                   "de", /* TODO: get country */
                                   bankCode,
                                   accountId,
                                   accountSuffix);
	if (acc) {
          DBG_NOTICE(AQHBCI_LOGDOMAIN,
                     "Account \"%s\" already exists",
                     accountId);
	  accCreated=0;
	  rv=AB_Banking_BeginExclUseAccount(ab, acc);
	  if (rv<0) {
	    DBG_ERROR(AQHBCI_LOGDOMAIN, "here (%d)", rv);
	    mayModifyAcc=0;
	  }
	  else
	    mayModifyAcc=1;
	}
        else {
          DBG_NOTICE(AQHBCI_LOGDOMAIN, "Creating account \"%s\"", accountId);
	  accCreated=1;
	  mayModifyAcc=1;
          acc=AB_Banking_CreateAccount(ab, AH_PROVIDER_NAME);
          assert(acc);
          AB_Account_SetCountry(acc, "de");
          AB_Account_SetBankCode(acc, bankCode);
	  AB_Account_SetAccountNumber(acc, accountId);
          DBG_NOTICE(AQHBCI_LOGDOMAIN,
                     "Setting user \"%s\" for account \"%s\"",
                     AB_User_GetUserId(u),
                     accountId);
          AB_Account_SetUser(acc, j->user);
          AB_Account_SetSelectedUser(acc, j->user);
        }

	if (mayModifyAcc) {
	  /* modify account */
	  if (accountName)
	    AB_Account_SetAccountName(acc, accountName);
	  if (userName)
	    AB_Account_SetOwnerName(acc, userName);
	  if (iban)
	    AB_Account_SetIBAN(acc, iban);
          if (accountSuffix && *accountSuffix)
            AB_Account_SetSubAccountId(acc, accountSuffix);

	  if (strcasecmp(GWEN_DB_GroupName(dbRd), "AccountData2")==0) {
	    /* KTV in version 2 available */
	    AH_Account_AddFlags(acc, AH_BANK_FLAGS_KTV2);
	    DBG_NOTICE(AQHBCI_LOGDOMAIN, "Extended account information is available");
	    GWEN_Gui_ProgressLog(0,
				 GWEN_LoggerLevel_Notice,
				 I18N("Extended account information available"));
	  }
	  else {
	    AH_Account_SubFlags(acc, AH_BANK_FLAGS_KTV2);
	    DBG_WARN(AQHBCI_LOGDOMAIN, "Extended account information is not available, some jobs might not work");
	    GWEN_Gui_ProgressLog(0,
				 GWEN_LoggerLevel_Warning,
				 I18N("Extended account information is not available, some jobs might not work"));
	  }

	  /* set bank name */
	  bpd=AH_User_GetBpd(j->user);
	  if (bpd) {
	    const char *s;

	    s=AH_Bpd_GetBankName(bpd);
	    if (s)
	      AB_Account_SetBankName(acc, s);
	  }

	  /* get UPD jobs */
	  dbUpd=AH_User_GetUpd(j->user);
	  assert(dbUpd);
	  dbUpd=GWEN_DB_GetGroup(dbUpd, GWEN_DB_FLAGS_OVERWRITE_GROUPS,
				 accountId);
	  assert(dbUpd);
	  gr=GWEN_DB_GetFirstGroup(dbRd);
	  while(gr) {
	    if (strcasecmp(GWEN_DB_GroupName(gr), "updjob")==0) {
	      /* found an upd job */
	      DBG_NOTICE(AQHBCI_LOGDOMAIN, "Adding UPD job");
	      GWEN_DB_AddGroup(dbUpd, GWEN_DB_Group_dup(gr));
	    }
	    gr=GWEN_DB_GetNextGroup(gr);
	  } /* while */
	}

        if (accCreated)
	  AB_Banking_AddAccount(ab, acc);
	else {
	  if (mayModifyAcc) {
	    rv=AB_Banking_EndExclUseAccount(ab, acc, 0);
	    if (rv<0) {
	      DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
	      AB_Banking_EndExclUseAccount(ab, acc, 1); /* abort */
	    }
	  }
	}
      } /* if accountData */

      if (strcasecmp(GWEN_DB_GroupName(dbRd), "BankMsg")==0){
        const char *subject;
        const char *text;

        DBG_NOTICE(AQHBCI_LOGDOMAIN, "Found a bank message");
	GWEN_Gui_ProgressLog(0,
			     GWEN_LoggerLevel_Notice,
			     I18N("Bank message received"));
        subject=GWEN_DB_GetCharValue(dbRd, "subject", 0, "(Kein Betreff)");
        text=GWEN_DB_GetCharValue(dbRd, "text", 0, 0);
        if (subject && text) {
	  AB_MESSAGE *amsg;
          GWEN_TIME *ti;

          ti=GWEN_CurrentTime();
	  amsg=AB_Message_new();
	  AB_Message_SetSubject(amsg, subject);
	  AB_Message_SetText(amsg, text);
	  AB_Message_SetDateReceived(amsg, ti);
	  GWEN_Time_free(ti);
	  AB_Message_SetUserId(amsg, AB_User_GetUniqueId(u));
	  AB_Message_List_Add(amsg, j->messages);

	  if (1) {
	    GWEN_DB_NODE *dbTmp;

            /* save message, later this will no longer be necessary */
	    dbTmp=GWEN_DB_Group_new("bank message");
	    GWEN_DB_SetCharValue(dbTmp, GWEN_DB_FLAGS_OVERWRITE_VARS,
				 "subject", subject);
	    GWEN_DB_SetCharValue(dbTmp, GWEN_DB_FLAGS_OVERWRITE_VARS,
				 "text", text);
	    if (AH_HBCI_SaveMessage(h, j->user, dbTmp)) {
	      DBG_ERROR(AQHBCI_LOGDOMAIN, "Could not save this message:");
	      GWEN_DB_Dump(dbTmp, 2);
	    }
	    GWEN_DB_Group_free(dbTmp);
	  }

        } /* if subject and text given */
      } /* if bank msg */


    } /* if response data found */
    dbCurr=GWEN_DB_GetNextGroup(dbCurr);
  } /* while */

  DBG_NOTICE(AQHBCI_LOGDOMAIN, "Finished.");
  return 0;
}



int AH_Job_CommitSystemData(AH_JOB *j, int doLock) {
  AB_USER *u;
  AB_BANKING *ab;
  int rv, rv2;

  u=j->user;
  assert(u);

  ab=AH_Job_GetBankingApi(j);
  assert(ab);

  /* lock user */
  if (doLock) {
    rv=AB_Banking_BeginExclUseUser(ab, u);
    if (rv<0) {
      DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
      return rv;
    }
  }

  /* commit data */
  rv2=AH_Job__CommitSystemData(j, doLock);

  if (doLock) {
    /* unlock user */
    rv=AB_Banking_EndExclUseUser(ab, u, 0);
    if (rv<0) {
      DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
      AB_Banking_EndExclUseUser(ab, u, 1); /* abandon */
      return rv;
    }
  }

  return rv2;
}



int AH_Job_HasItanResult(AH_JOB *j) {
  GWEN_DB_NODE *dbCurr;

  assert(j);
  assert(j->usage);

  dbCurr=GWEN_DB_GetFirstGroup(j->jobResponses);
  while(dbCurr) {
    GWEN_DB_NODE *dbRd;

    dbRd=GWEN_DB_GetGroup(dbCurr, GWEN_PATH_FLAGS_NAMEMUSTEXIST, "data");
    if (dbRd) {
      dbRd=GWEN_DB_GetFirstGroup(dbRd);
    }
    if (dbRd) {
      if (strcasecmp(GWEN_DB_GroupName(dbRd), "SegResult")==0){
        GWEN_DB_NODE *dbRes;

        dbRes=GWEN_DB_GetFirstGroup(dbRd);
        while(dbRes) {
          if (strcasecmp(GWEN_DB_GroupName(dbRes), "result")==0) {
            int code;
//            const char *text;
  
            code=GWEN_DB_GetIntValue(dbRes, "resultcode", 0, 0);
//            text=GWEN_DB_GetCharValue(dbRes, "text", 0, 0);
            if (code==3920) {
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



int AH_Job_DefaultProcessHandler(AH_JOB *j){
  assert(j);
  assert(j->usage);
  if (j->flags & AH_JOB_FLAGS_PROCESSED) {
    DBG_WARN(AQHBCI_LOGDOMAIN, "Already processed job \"%s\"", j->name);
    return 0;
  }
  return 0;
}



int AH_Job_DefaultCommitHandler(AH_JOB *j, int doLock){
  int rv;

  assert(j);
  assert(j->usage);
  if (j->flags & AH_JOB_FLAGS_COMMITTED) {
    DBG_WARN(AQHBCI_LOGDOMAIN, "Already committed job \"%s\"", j->name);
    return 0;
  }
  rv=AH_Job_CommitSystemData(j, doLock);
  j->flags|=AH_JOB_FLAGS_COMMITTED;
  return rv;
}




AH_JOB *AH_Job__freeAll_cb(AH_JOB *j, void *userData) {
  assert(j);
  assert(j->usage);
  AH_Job_free(j);
  return 0;
}



void AH_Job_List2_FreeAll(AH_JOB_LIST2 *jl){
  AH_Job_List2_ForEach(jl, AH_Job__freeAll_cb, 0);
  AH_Job_List2_free(jl);
}



AH_HBCI *AH_Job_GetHbci(const AH_JOB *j){
  assert(j);
  assert(j->usage);

  return AH_User_GetHbci(j->user);
}



AB_BANKING *AH_Job_GetBankingApi(const AH_JOB *j){
  AH_HBCI *hbci;

  assert(j);
  assert(j->usage);
  hbci=AH_Job_GetHbci(j);
  assert(hbci);
  return AH_HBCI_GetBankingApi(hbci);
}



AH_RESULT_LIST *AH_Job_GetSegResults(const AH_JOB *j){
  assert(j);
  assert(j->usage);
  return j->segResults;
}



AH_RESULT_LIST *AH_Job_GetMsgResults(const AH_JOB *j){
  assert(j);
  assert(j->usage);
  return j->msgResults;
}



const char *AH_Job_GetExpectedSigner(const AH_JOB *j){
  assert(j);
  assert(j->usage);
  return j->expectedSigner;
}



void AH_Job_SetExpectedSigner(AH_JOB *j, const char *s){
  assert(j);
  assert(j->usage);
  free(j->expectedSigner);
  if (s) j->expectedSigner=strdup(s);
  else j->expectedSigner=0;
}



const char *AH_Job_GetExpectedCrypter(const AH_JOB *j){
  assert(j);
  assert(j->usage);
  return j->expectedCrypter;
}



void AH_Job_SetExpectedCrypter(AH_JOB *j, const char *s){
  assert(j);
  assert(j->usage);
  free(j->expectedCrypter);
  if (s) j->expectedCrypter=strdup(s);
  else j->expectedCrypter=0;
}



int AH_Job_CheckEncryption(AH_JOB *j, GWEN_DB_NODE *dbRsp) {
  if (AH_User_GetCryptMode(j->user)==AH_CryptMode_Pintan) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "Not checking security in PIN/TAN mode");
  }
  else {
    GWEN_DB_NODE *dbSecurity;
    const char *s;
  
    assert(j);
    assert(j->usage);
    assert(dbRsp);
    dbSecurity=GWEN_DB_GetGroup(dbRsp, GWEN_PATH_FLAGS_NAMEMUSTEXIST,
				"security");
    if (!dbSecurity) {
      DBG_ERROR(AQHBCI_LOGDOMAIN,
		"No security settings, should not happen");
      GWEN_Gui_ProgressLog(
			     0,
			     GWEN_LoggerLevel_Error,
			     I18N("Response without security info (internal)"));
      return AB_ERROR_SECURITY;
    }
  
    s=GWEN_DB_GetCharValue(dbSecurity, "crypter", 0, 0);
    if (s) {
      if (*s=='!' || *s=='?') {
	DBG_ERROR(AQHBCI_LOGDOMAIN,
		  "Encrypted with invalid key (%s)", s);
	GWEN_Gui_ProgressLog(
			       0,
			       GWEN_LoggerLevel_Error,
			       I18N("Response encrypted with invalid key"));
	return AB_ERROR_SECURITY;
      }
    }
    if (j->expectedCrypter) {
      /* check crypter */
      if (!s) {
	DBG_ERROR(AQHBCI_LOGDOMAIN,
		  "Response is not encrypted (but expected to be)");
	GWEN_Gui_ProgressLog(0,
			     GWEN_LoggerLevel_Error,
			     I18N("Response is not encrypted as expected"));
	return AB_ERROR_SECURITY;
  
      }
  
      if (strcasecmp(s, j->expectedCrypter)!=0) {
	DBG_WARN(AQHBCI_LOGDOMAIN,
		 "Not encrypted with the expected key "
		 "(exp: \"%s\", is: \"%s\"",
		 j->expectedCrypter, s);
	/*
	GWEN_Gui_ProgressLog(
			       0,
			       GWEN_LoggerLevel_Error,
			       I18N("Response not encrypted with expected key"));
	return AB_ERROR_SECURITY;
	*/
      }
      else {
	DBG_INFO(AQHBCI_LOGDOMAIN, "Encrypted as expected");
      }
    }
    else {
      DBG_INFO(AQHBCI_LOGDOMAIN, "No encryption expected");
    }
  }

  return 0;
}



int AH_Job_CheckSignature(AH_JOB *j, GWEN_DB_NODE *dbRsp) {
  if (AH_User_GetCryptMode(j->user)==AH_CryptMode_Pintan) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "Not checking signature in PIN/TAN mode");
  }
  else {
    GWEN_DB_NODE *dbSecurity;
    int i;
    uint32_t uFlags;
  
    assert(j);
    assert(j->usage);
  
    uFlags=AH_User_GetFlags(j->user);
  
    assert(dbRsp);
    dbSecurity=GWEN_DB_GetGroup(dbRsp, GWEN_PATH_FLAGS_NAMEMUSTEXIST,
				"security");
    if (!dbSecurity) {
      DBG_ERROR(AQHBCI_LOGDOMAIN,
		"No security settings, should not happen");
      GWEN_Gui_ProgressLog(
			     0,
			     GWEN_LoggerLevel_Error,
			     I18N("Response without security info (internal)"));
      return GWEN_ERROR_GENERIC;
    }
  
    /* check for invalid signers */
    for (i=0; ; i++) {
      const char *s;
  
      s=GWEN_DB_GetCharValue(dbSecurity, "signer", i, 0);
      if (!s)
	break;
      if (*s=='!') {
	DBG_ERROR(AQHBCI_LOGDOMAIN,
		  "Invalid signature found, will not tolerate it");
	GWEN_Gui_ProgressLog(0,
			     GWEN_LoggerLevel_Error,
			     I18N("Invalid bank signature"));
	return AB_ERROR_SECURITY;
      }
    } /* for */
  
    if (j->expectedSigner && !(uFlags & AH_USER_FLAGS_BANK_DOESNT_SIGN)) {
      /* check signer */
      for (i=0; ; i++) {
	const char *s;
  
	s=GWEN_DB_GetCharValue(dbSecurity, "signer", i, 0);
	if (!s) {
	  DBG_ERROR(AQHBCI_LOGDOMAIN,
		    "Not signed by expected signer (%d)", i);
	  GWEN_Gui_ProgressLog(0,
			       GWEN_LoggerLevel_Error,
			       I18N("Response not signed by the bank"));
	  if (i==0) {
	    int but;
  
	    /* check whether the user want's to accept the unsigned message */
	    but=GWEN_Gui_MessageBox(GWEN_GUI_MSG_FLAGS_TYPE_WARN |
				    GWEN_GUI_MSG_FLAGS_CONFIRM_B1 |
				    GWEN_GUI_MSG_FLAGS_SEVERITY_DANGEROUS,
	       I18N("Security Warning"),
	       I18N(
  "The HBCI response of the bank has not been signed by the bank, \n"
  "contrary to what has been expected. This can be the case because the \n"
  "bank just stopped signing their HBCI responses. This error message \n"
  "would also occur if there were a replay attack against your computer \n"
  "in progress right now, which is probably quite unlikely. \n"
  " \n"
  "Please contact your bank and ask them whether their HBCI server \n"
  "stopped signing the HBCI responses. If the bank is concerned about \n"
  "your security, it should not stop signing the HBCI responses. \n"
  " \n"
  "Do you nevertheless want to accept this response this time or always?"
  "<html><p>"
  "The HBCI response of the bank has not been signed by the bank, \n"
  "contrary to what has been expected. This can be the case because the \n"
  "bank just stopped signing their HBCI responses. This error message \n"
  "would also occur if there were a replay attack against your computer \n"
  "in progress right now, which is probably quite unlikely. \n"
  "</p><p>"
  "Please contact your bank and ask them whether their HBCI server \n"
  "stopped signing the HBCI responses. If the bank is concerned about \n"
  "your security, it should not stop signing the HBCI responses. \n"
  "</p><p>"
  "Do you nevertheless want to accept this response this time or always?"
  "</p></html>"
  ),
	       I18N("Accept this time"),
	       I18N("Accept always"),
	       I18N("Abort"), 0);
	    if (but==1) {
	      GWEN_Gui_ProgressLog(0,
				   GWEN_LoggerLevel_Notice,
				   I18N("User accepts this unsigned "
					"response"));
	      AH_Job_SetExpectedSigner(j, 0);
	      break;
	    }
	    else if (but==2) {
	      GWEN_Gui_ProgressLog(0,
				   GWEN_LoggerLevel_Notice,
				   I18N("User accepts all further unsigned "
					"responses"));
	      AH_User_AddFlags(j->user, AH_USER_FLAGS_BANK_DOESNT_SIGN);
	      AH_Job_SetExpectedSigner(j, 0);
	      break;
	    }
	    else {
	      GWEN_Gui_ProgressLog(0,
				   GWEN_LoggerLevel_Error,
				   I18N("Aborted"));
	      return AB_ERROR_SECURITY;
	    }
	  }
	  else {
	    int ii;
  
	    DBG_ERROR(AQHBCI_LOGDOMAIN,
		      "Job signed with unexpected key(s)"
		      "(was expecting \"%s\"):",
		      j->expectedSigner);
	    for (ii=0; ; ii++) {
	      s=GWEN_DB_GetCharValue(dbSecurity, "signer", ii, 0);
	      if (!s)
		break;
	      DBG_ERROR(AQHBCI_LOGDOMAIN,
			"Signed unexpectedly with key \"%s\"", s);
	    }
	    return AB_ERROR_SECURITY;
	  }
	}
	else {
	  if (strcasecmp(s, j->expectedSigner)==0) {
	    DBG_INFO(AQHBCI_LOGDOMAIN,
		     "Jobs signed as expected with \"%s\"",
		     j->expectedSigner);
	    break;
	  }
	  else if (*s!='!' && *s!='?') {
	    DBG_INFO(AQHBCI_LOGDOMAIN,
		     "Signer name does not match expected name (%s!=%s), "
		     "but we accept it anyway",
		     s, j->expectedSigner);
	    break;
	  }
	}
      } /* for */
      DBG_INFO(AQHBCI_LOGDOMAIN, "Signature check ok");
    }
    else {
      DBG_INFO(AQHBCI_LOGDOMAIN, "No signature expected");
    }
  }
  return 0;
}



const char *AH_Job_GetUsedTan(const AH_JOB *j){
  assert(j);
  assert(j->usage);
  return j->usedTan;
}



void AH_Job_SetUsedTan(AH_JOB *j, const char *s){
  assert(j);
  assert(j->usage);

  DBG_INFO(AQHBCI_LOGDOMAIN, "Changing TAN in job [%s](%08x) from [%s] to [%s]",
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



void AH_Job_Log(AH_JOB *j, GWEN_LOGGER_LEVEL ll, const char *txt) {
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



const GWEN_STRINGLIST *AH_Job_GetLogs(const AH_JOB *j) {
  assert(j);
  return j->log;
}



GWEN_STRINGLIST *AH_Job_GetChallengeParams(const AH_JOB *j) {
  assert(j);
  return j->challengeParams;
}



void AH_Job_ClearChallengeParams(AH_JOB *j) {
  assert(j);
  GWEN_StringList_Clear(j->challengeParams);
}



void AH_Job_AddChallengeParam(AH_JOB *j, const char *s) {
  assert(j);
  GWEN_StringList_AppendString(j->challengeParams, s, 0, 0);
}



const AB_VALUE *AH_Job_GetChallengeValue(const AH_JOB *j) {
  assert(j);
  return j->challengeValue;
}



void AH_Job_SetChallengeValue(AH_JOB *j, const AB_VALUE *v) {
  assert(j);
  AB_Value_free(j->challengeValue);
  if (v) j->challengeValue=AB_Value_dup(v);
  else j->challengeValue=NULL;
}



void AH_Job_ValueToChallengeString(const AB_VALUE *v, GWEN_BUFFER *buf) {
  AB_Value_toHbciString(v, buf);
}



int AH_Job_GetTransferCount(AH_JOB *j) {
  assert(j);
  return j->transferCount;
}



void AH_Job_IncTransferCount(AH_JOB *j) {
  assert(j);
  j->transferCount++;
}



int AH_Job_GetMaxTransfers(AH_JOB *j) {
  assert(j);
  return j->maxTransfers;
}



void AH_Job_SetMaxTransfers(AH_JOB *j, int i) {
  assert(j);
  j->maxTransfers=i;
}



AB_TRANSACTION_LIST *AH_Job_GetTransferList(const AH_JOB *j) {
  assert(j);
  return j->transferList;
}



AB_TRANSACTION *AH_Job_GetFirstTransfer(const AH_JOB *j) {
  assert(j);
  if (j->transferList==NULL)
    return NULL;

  return AB_Transaction_List_First(j->transferList);
}



void AH_Job_AddTransfer(AH_JOB *j, AB_TRANSACTION *t) {
  assert(j);
  if (j->transferList==NULL)
    j->transferList=AB_Transaction_List_new();

  AB_Transaction_List_Add(t, j->transferList);
  j->transferCount++;
}



static int AH_Job__SepaProfileSupported(GWEN_DB_NODE *profile,
                                        const GWEN_STRINGLIST *descriptors) {
  GWEN_STRINGLISTENTRY *se;
  char pattern[13];
  const char *s;

  /* patterns shall always have the form *xxx.yyy.zz* */
  pattern[0]=pattern[11]='*';
  pattern[12]='\0';

  /* Well formed type strings are exactly 10 characters long. Others
   * will either not match or be rejected by the exporter. */
  strncpy(pattern+1, GWEN_DB_GetCharValue(profile, "type", 0, ""), 10);
  se=GWEN_StringList_FirstEntry(descriptors);
  while(se) {
    s=GWEN_StringListEntry_Data(se);
    if (s && GWEN_Text_ComparePattern(s, pattern, 1)!=-1) {
      /* record the descriptor matching this profile */
      GWEN_DB_SetCharValue(profile, GWEN_DB_FLAGS_OVERWRITE_VARS,
                           "descriptor", s);
      break;
    }
    se=GWEN_StringListEntry_Next(se);
  }
  if (se)
    return 1;
  else
    return 0;
}



static int AH_Job__SortSepaProfiles(const void *a, const void *b) {
  GWEN_DB_NODE **ppa=(GWEN_DB_NODE **)a;
  GWEN_DB_NODE **ppb=(GWEN_DB_NODE **)b;
  GWEN_DB_NODE *pa=*ppa;
  GWEN_DB_NODE *pb=*ppb;
  int res;

  /* This function is supposed to return a list of profiles in order
   * of decreasing precedence. */
  res=strcmp(GWEN_DB_GetCharValue(pb, "type", 0, ""),
             GWEN_DB_GetCharValue(pa, "type", 0, ""));
  if (res)
    return res;

  res=strcmp(GWEN_DB_GetCharValue(pb, "name", 0, ""),
             GWEN_DB_GetCharValue(pa, "name", 0, ""));

  return res;
}



GWEN_DB_NODE *AH_Job_FindSepaProfile(AH_JOB *j, const char *type,
                                     const char *name) {
  const GWEN_STRINGLIST *descriptors;
  GWEN_DB_NODE *dbProfiles;

  assert(j);

  if (j->sepaDescriptors)
    descriptors=j->sepaDescriptors;
  else
    descriptors=AH_User_GetSepaDescriptors(j->user);
  if (GWEN_StringList_Count(descriptors)==0) {
    DBG_ERROR(AQHBCI_LOGDOMAIN,
              "No SEPA descriptor found, please update your account information");
    return NULL;
  }

  if (name) {
    GWEN_DB_NODE *profile;

    profile=AB_Banking_GetImExporterProfile(AH_Job_GetBankingApi(j),
                                            "sepa", name);
    if (!profile) {
      DBG_ERROR(AQHBCI_LOGDOMAIN,
                "Profile \"%s\" not available", name);
      return NULL;
    }
    if (GWEN_Text_ComparePattern(GWEN_DB_GetCharValue(profile, "type", 0, ""),
                                 type, 1)==-1) {
      DBG_ERROR(AQHBCI_LOGDOMAIN,
                "Profile \"%s\" does not match type speecification (\"%s\")",
                name, type);
      return NULL;
    }
    if (!AH_Job__SepaProfileSupported(profile, descriptors)) {
      DBG_ERROR(AQHBCI_LOGDOMAIN,
                "Profile \"%s\" not supported by bank server", name);
      return NULL;
    }
    j->sepaProfile=profile;
    return profile;
  }

  if (!type)
    return j->sepaProfile;

  if (j->sepaProfile) {
    if (GWEN_Text_ComparePattern(GWEN_DB_GetCharValue(j->sepaProfile, "type",
                                                      0, ""),
                                 type, 1)!=-1)
      return j->sepaProfile;
    else {
      GWEN_DB_Group_free(j->sepaProfile);
      j->sepaProfile=NULL;
    }
  }

  dbProfiles=AB_Banking_GetImExporterProfiles(AH_Job_GetBankingApi(j), "sepa");
  if (dbProfiles) {
    GWEN_DB_NODE *n, *nn;
    unsigned int pCount=0;

    n=GWEN_DB_GetFirstGroup(dbProfiles);
    while(n) {
      nn=n;
      n=GWEN_DB_GetNextGroup(n);

      if (GWEN_Text_ComparePattern(GWEN_DB_GetCharValue(nn, "type", 0, ""),
                                   type, 1)!=-1 &&
          AH_Job__SepaProfileSupported(nn, descriptors))
        pCount++;
      else {
        GWEN_DB_UnlinkGroup(nn);
        GWEN_DB_Group_free(nn);
      }
    }

    if (pCount) {
      GWEN_DB_NODE **orderedProfiles;
      unsigned int i;

      orderedProfiles=malloc(pCount*sizeof(GWEN_DB_NODE *));
      assert(orderedProfiles);
      n=GWEN_DB_GetFirstGroup(dbProfiles);
      for (i=0; i<pCount && n; i++) {
        orderedProfiles[i]=n;
        n=GWEN_DB_GetNextGroup(n);
      }
      assert(i==pCount && !n);
      qsort(orderedProfiles, pCount, sizeof(GWEN_DB_NODE *),
            AH_Job__SortSepaProfiles);
      GWEN_DB_UnlinkGroup(orderedProfiles[0]);
      j->sepaProfile=orderedProfiles[0];
      free(orderedProfiles);
    }
    else {
      DBG_ERROR(AQHBCI_LOGDOMAIN,
                "No supported SEPA format found for job \"%s\"", j->name);
    }
    GWEN_DB_Group_free(dbProfiles);
  }
  else {
    DBG_ERROR(AQHBCI_LOGDOMAIN,
              "No SEPA profiles found");
  }

  return j->sepaProfile;
}



