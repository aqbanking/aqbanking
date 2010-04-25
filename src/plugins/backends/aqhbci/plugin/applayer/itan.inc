/***************************************************************************
    begin       : Mon Mar 01 2004
    copyright   : (C) 2004-2010 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

/* This file is included by outbox.c */


#include "message_l.h"

#include <gwenhywfar/mdigest.h>


int AH_Outbox__CBox__Hash(int mode,
			  const uint8_t *p,
			  unsigned int l,
			  AH_MSG *msg) {
  GWEN_MDIGEST *md=NULL;
  int rv;
  GWEN_BUFFER *hbuf;

  DBG_DEBUG(AQHBCI_LOGDOMAIN, "Hashmode: %d", mode);

  switch(mode) {
  case 0:
    DBG_NOTICE(AQHBCI_LOGDOMAIN, "No ITAN hash mode, assuming RMD160");
    /* fall through */
  case 1:  /* RMD160 over buffer */
    DBG_INFO(AQHBCI_LOGDOMAIN, "Using RMD160");
    md=GWEN_MDigest_Rmd160_new();
    if (md==NULL) {
      DBG_ERROR(AQHBCI_LOGDOMAIN, "Could not create MD RMD160");
    }
    break;

  case 2:  /* SHA over buffer */
    DBG_INFO(AQHBCI_LOGDOMAIN, "Using SHA1");
    md=GWEN_MDigest_Sha1_new();
    if (md==NULL) {
      DBG_ERROR(AQHBCI_LOGDOMAIN, "Could not create MD SHA1");
    }
    break;

  default: /* invalid mode */
    DBG_ERROR(AQHBCI_LOGDOMAIN,
	      "Invalid ITAN hash mode \"%d\"",
	      mode);
    return GWEN_ERROR_INVALID;
  }

  if (md==NULL) {
    DBG_ERROR(AQHBCI_LOGDOMAIN,
	      "No message digestion algo (mode %d)", mode);
    return GWEN_ERROR_INVALID;
  }

  rv=GWEN_MDigest_Begin(md);
  if (rv<0) {
    GWEN_MDigest_free(md);
    return rv;
  }
  rv=GWEN_MDigest_Update(md, p, l);
  if (rv<0) {
    GWEN_MDigest_free(md);
    return rv;
  }
  rv=GWEN_MDigest_End(md);
  if (rv<0) {
    GWEN_MDigest_free(md);
    return rv;
  }

  hbuf=GWEN_Buffer_new(0, 32, 0, 1);
  GWEN_Buffer_AppendBytes(hbuf,
			  (const char*)GWEN_MDigest_GetDigestPtr(md),
			  GWEN_MDigest_GetDigestSize(md));
  GWEN_MDigest_free(md);
  AH_Msg_SetItanHashBuffer(msg, hbuf);

  DBG_NOTICE(AQHBCI_LOGDOMAIN, "Hashed job segment");

  return 0;
}



int AH_Outbox__CBox_JobToMessage(AH_JOB *j, AH_MSG *msg){
  AB_USER *user;
  unsigned int firstSeg;
  unsigned int lastSeg;
  GWEN_DB_NODE *jargs;
  GWEN_XMLNODE *jnode;
  GWEN_BUFFER *msgBuf;
  uint32_t startPos;
  uint32_t endPos;

  DBG_NOTICE(AQHBCI_LOGDOMAIN, "Encoding job \"%s\"", AH_Job_GetName(j));
  user=AH_Job_GetUser(j);
  assert(user);

  /* setup message */
  AH_Msg_SetHbciVersion(msg, AH_User_GetHbciVersion(user));
  if (AH_Job_GetFlags(j) & AH_JOB_FLAGS_NEEDTAN) {
    DBG_NOTICE(AQHBCI_LOGDOMAIN, "Queue needs a TAN");
  }
  else {
    DBG_NOTICE(AQHBCI_LOGDOMAIN, "Jobs doesn't need a TAN");
  }
  AH_Msg_SetNeedTan(msg,
                    (AH_Job_GetFlags(j) & AH_JOB_FLAGS_NEEDTAN));

  /* copy signers */
  if (AH_Job_GetFlags(j) & AH_JOB_FLAGS_SIGN) {
    GWEN_STRINGLISTENTRY *se;

    se=GWEN_StringList_FirstEntry(AH_Job_GetSigners(j));
    if (!se) {
      DBG_ERROR(AQHBCI_LOGDOMAIN, "Signatures needed but no signer given");
      return GWEN_ERROR_INVALID;
    }
    while(se) {
      AH_Msg_AddSignerId(msg, GWEN_StringListEntry_Data(se));
      se=GWEN_StringListEntry_Next(se);
    } /* while */
  }

  /* copy crypter */
  if (AH_Job_GetFlags(j) & AH_JOB_FLAGS_CRYPT) {
    /* The name doesn't matter here, since jobs are only used by clients
     * and the client code for getMedium always uses the name of the dialog
     * owner instead of the name from the keyspec when retrieving the medium
     * for encryption.
     */
    AH_Msg_SetCrypterId(msg, "owner");
  }

  /* get arguments and XML node */
  jargs=AH_Job_GetArguments(j);
  jnode=AH_Job_GetXmlNode(j);
  if (strcasecmp(GWEN_XMLNode_GetData(jnode), "message")==0) {
    const char *s;

    s=GWEN_XMLNode_GetProperty(jnode, "name", 0);
    if (s) {
      DBG_NOTICE(AQHBCI_LOGDOMAIN,
		 "Getting for message specific data (%s)", s);
      jargs=GWEN_DB_GetGroup(jargs, GWEN_PATH_FLAGS_NAMEMUSTEXIST, s);
      if (!jargs) {
	DBG_NOTICE(AQHBCI_LOGDOMAIN, "No message specific data");
	jargs=AH_Job_GetArguments(j);
      }
    }
  }

  /* add job node to message */
  firstSeg=AH_Msg_GetCurrentSegmentNumber(msg);
  msgBuf=AH_Msg_GetBuffer(msg);
  assert(msgBuf);
  startPos=GWEN_Buffer_GetPos(msgBuf);
  lastSeg=AH_Msg_AddNode(msg, jnode, jargs);
  if (!lastSeg) {
    DBG_NOTICE(AQHBCI_LOGDOMAIN, "Could not encode job \"%s\"",
	     AH_Job_GetName(j));
    AH_Job_SetStatus(j, AH_JobStatusError);
    return GWEN_ERROR_INTERNAL;
  }
  else {
    AH_Job_SetFirstSegment(j, firstSeg);
    AH_Job_SetLastSegment(j, lastSeg);

    /* iTAN management */
    if (AH_Msg_GetItanHashBuffer(msg)==0) {
      int rv;

      endPos=GWEN_Buffer_GetPos(msgBuf);
      rv=AH_Outbox__CBox__Hash(AH_Msg_GetItanHashMode(msg),
			       (const uint8_t*)GWEN_Buffer_GetStart(msgBuf)+startPos,
			       endPos-startPos,
			       msg);
      if (rv) {
	DBG_ERROR(AQHBCI_LOGDOMAIN,
		  "Could not hash data (%d)", rv);
	AH_Job_SetStatus(j, AH_JobStatusError);
	return rv;
      }
    }

    if (AH_Job_GetStatus(j)!=AH_JobStatusError) {
      DBG_NOTICE(AQHBCI_LOGDOMAIN, "Job \"%s\" encoded",
	       AH_Job_GetName(j));
      AH_Job_SetStatus(j, AH_JobStatusEncoded);
    }
  }

  return 0;
}



int AH_Outbox__CBox_Itan_SendMsg(AH_OUTBOX__CBOX *cbox,
				 AH_DIALOG *dlg,
				 AH_MSG *msg,
				 int timeout,
				 uint32_t guiid) {
  int rv;

  DBG_NOTICE(AQHBCI_LOGDOMAIN, "Sending queue");
  GWEN_Gui_ProgressLog(guiid,
		       GWEN_LoggerLevel_Info,
		       I18N("Sending queue"));
  rv=AH_Dialog_SendMessage(dlg, msg);
  if (rv) {
    DBG_NOTICE(AQHBCI_LOGDOMAIN, "Could not send message");
    GWEN_Gui_ProgressLog(guiid,
			 GWEN_LoggerLevel_Error,
			 I18N("Unable to send (network error)"));
    return rv;
  }
  DBG_NOTICE(AQHBCI_LOGDOMAIN, "Message sent");

  return 0;
}



int AH_Outbox__CBox_Itan(AH_OUTBOX__CBOX *cbox,
			 AH_DIALOG *dlg,
			 AH_JOBQUEUE *qJob,
			 int timeout,
			 uint32_t guiid){
  int rv;
  int process;

  process=AH_Dialog_GetItanProcessType(dlg);
  if (process==1)
    rv=AH_Outbox__CBox_Itan1(cbox, dlg, qJob, timeout, guiid);
  else if (process==2)
    rv=AH_Outbox__CBox_Itan2(cbox, dlg, qJob, timeout, guiid);
  else {
    DBG_ERROR(AQHBCI_LOGDOMAIN,
	      "iTAN method %d not supported", process);
    return GWEN_ERROR_INVALID;
  }

  return rv;
}



int AH_Outbox__CBox_SelectItanMode(AH_OUTBOX__CBOX *cbox,
				   AH_DIALOG *dlg) {
  AH_JOB *jTan;
  AB_USER *u;
  GWEN_DB_NODE *dbParams;
  GWEN_DB_NODE *dbMethod=0;
  int fn=0;
  int process=0;

  u=cbox->user;
  assert(u);

  /* test create HKTAN job */
  jTan=AH_Job_Tan_new(u, 0, 0);
  /* TODO!!!! Select HKTAN version, call AH_Dialog_SetTanJobVersion( */
  if (!jTan) {
    DBG_WARN(AQHBCI_LOGDOMAIN, "Job HKTAN not (yet?) available");
    AH_Dialog_SetItanMethod(dlg, 0);
    AH_Dialog_SetItanProcessType(dlg, 0);
    return 0;
  }

  fn=AH_User_GetSelectedTanMethod(u);
  DBG_INFO(AQHBCI_LOGDOMAIN, "Selected TAN method: %d", fn);
  dbParams=AH_Job_GetParams(jTan);
  assert(dbParams);

  if (fn && AH_User_HasTanMethod(u, fn)) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "Searching DB node for selected TAN method %d", fn);
    /* find DB_NODE of selected TanMethod */
    dbMethod=GWEN_DB_FindFirstGroup(dbParams, "tanMethod");
    while(dbMethod) {
      if (GWEN_DB_GetIntValue(dbMethod, "function", 0, 0)==fn) {
	process=GWEN_DB_GetIntValue(dbMethod, "process", 0, 0);
	if (process==1 || process==2) {
	  DBG_INFO(AQHBCI_LOGDOMAIN, "Found DB for selected TAN method %d (process: %d)",
		   fn, process);
	  break;
	}
	else {
	  DBG_NOTICE(AQHBCI_LOGDOMAIN,
		     "iTan process type \"%d\" not supported", process);
	  /* selected mode is not supported, choose another one later */
	  fn=0;
	  break;
	}
      }

      dbMethod=GWEN_DB_FindNextGroup(dbMethod, "tanMethod");
    }

    if (fn==0) {
      GWEN_Gui_ProgressLog(AH_Dialog_GetGuiId(dlg),
			   GWEN_LoggerLevel_Warning,
			   I18N("TAN method selected by user is no longer valid,"
				"please choose another one"));
    }
  }

  if (fn==0) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "Selected TAN method is 0, looking for a usable method");
    /* find DB_NODE of first supported TanMethod */
    dbMethod=GWEN_DB_FindFirstGroup(dbParams, "tanMethod");
    while(dbMethod) {
      fn=GWEN_DB_GetIntValue(dbMethod, "function", 0, 0);
      if (AH_User_HasTanMethod(u, fn) ||
	  AH_User_GetTanMethodCount(u)==0) {
	process=GWEN_DB_GetIntValue(dbMethod, "process", 0, 0);
	if (process==1 || process==2)
	  break;
	else {
	  DBG_NOTICE(AQHBCI_LOGDOMAIN,
		     "iTan process type \"%d\" not supported", process);
	}
      }
      else {
        DBG_INFO(AQHBCI_LOGDOMAIN, "TAN method %d not allowed for this user", fn);
      }

      dbMethod=GWEN_DB_FindNextGroup(dbMethod, "tanMethod");
    }
  }

  if (!dbMethod || !fn) {
    DBG_ERROR(AQHBCI_LOGDOMAIN,
	      "No matching iTAN mode found (fn=%d)", fn);
    AH_Job_free(jTan);
    return GWEN_ERROR_NOT_FOUND;
  }
  else {
    const char *s;
    const char *lm=I18N_NOOP("Selecting iTAN mode \"%s\"");
    char lbuf[256];

    s=GWEN_DB_GetCharValue(dbMethod, "methodName", 0, 0);
    if (!s || !*s)
      s=GWEN_DB_GetCharValue(dbMethod, "methodId", 0, 0);
    snprintf(lbuf, sizeof(lbuf), I18N(lm), s);
    lbuf[sizeof(lbuf)-1]=0;
    DBG_NOTICE(AQHBCI_LOGDOMAIN, "Selecting iTAN mode \"%s\"", s);
    GWEN_Gui_ProgressLog(0,
			 GWEN_LoggerLevel_Info,
			 lbuf);
    AH_Dialog_SetItanMethod(dlg, fn);
    AH_Dialog_SetItanProcessType(dlg, process);

    AH_Job_free(jTan);
    return 0;
  }
}



void AH_Outbox__CBox_CopyJobResultsToJobList(const AH_JOB *j,
					     const AH_JOB_LIST *qjl) {
  /* dispatch results from jTan to all other members of the queue */
  if (qjl) {
    AH_RESULT_LIST *rl;
  
    /* segment results */
    rl=AH_Job_GetSegResults(j);
    if (rl) {
      AH_RESULT *or;
  
      or=AH_Result_List_First(rl);
      while(or) {
        AH_JOB *qj;
  
        qj=AH_Job_List_First(qjl);
        while(qj) {
	  if (qj!=j) {
            AH_RESULT *nr;
  
            nr=AH_Result_dup(or);
            AH_Result_List_Add(nr, AH_Job_GetSegResults(qj));
          }
          qj=AH_Job_List_Next(qj);
        }
  
        or=AH_Result_List_Next(or);
      } /* while or */
    } /* if rl */
  } /* if qjl */
}





