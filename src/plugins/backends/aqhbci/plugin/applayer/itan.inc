/***************************************************************************
 $RCSfile$
                             -------------------
    cvs         : $Id: adminjobs.c 950 2006-02-22 19:39:02Z aquamaniac $
    begin       : Mon Mar 01 2004
    copyright   : (C) 2004 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

/* This file is included by outbox.c */


#include "message_l.h"
#include "medium_l.h"

#include <gwenhywfar/md.h>




int AH_Outbox__CBox_JobToMessage(AH_JOB *j, AH_MSG *msg){
  AB_USER *user;
  unsigned int firstSeg;
  unsigned int lastSeg;
  GWEN_DB_NODE *jargs;
  GWEN_XMLNODE *jnode;
  GWEN_BUFFER *msgBuf;
  GWEN_TYPE_UINT32 startPos;
  GWEN_TYPE_UINT32 endPos;

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
      return AB_ERROR_INVALID;
    }
    while(se) {
      GWEN_KEYSPEC *ks;

      ks=GWEN_KeySpec_new();
      GWEN_KeySpec_SetKeyType(ks, "RSA");
      GWEN_KeySpec_SetOwner(ks, GWEN_StringListEntry_Data(se));
      GWEN_KeySpec_SetKeyName(ks, "S");
      GWEN_KeySpec_SetNumber(ks, 999);
      GWEN_KeySpec_SetVersion(ks, 999);
      AH_Msg_AddSigner(msg, ks);
      GWEN_KeySpec_free(ks);
      se=GWEN_StringListEntry_Next(se);
    } /* while */
  }

  /* copy crypter */
  if (AH_Job_GetFlags(j) & AH_JOB_FLAGS_CRYPT) {
    GWEN_KEYSPEC *ks;

    ks=GWEN_KeySpec_new();
    GWEN_KeySpec_SetKeyType(ks, "RSA");
    /* The name doesn't matter here, since jobs are only used by clients
     * and the client code for getMedium always uses the name of the dialog
     * owner instead of the name from the keyspec when retrieving the medium
     * for encryption.
     */
    GWEN_KeySpec_SetOwner(ks, "");
    GWEN_KeySpec_SetKeyName(ks, "V");
    GWEN_KeySpec_SetNumber(ks, 999);
    GWEN_KeySpec_SetVersion(ks, 999);
    AH_Msg_SetCrypter(msg, ks);
    GWEN_KeySpec_free(ks);
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
  }
  else {
    AH_Job_SetFirstSegment(j, firstSeg);
    AH_Job_SetLastSegment(j, lastSeg);

    /* iTAN management */
    if (AH_Msg_GetItanHashBuffer(msg)==0) {
      endPos=GWEN_Buffer_GetPos(msgBuf);
      switch(AH_Msg_GetItanHashMode(msg)) {
      case 0:
	DBG_NOTICE(AQHBCI_LOGDOMAIN, "No ITAN hash mode");
	break;

      case 1: { /* RMD160 over buffer */
	GWEN_BUFFER *hbuf;

	hbuf=GWEN_Buffer_new(0, 32, 0, 1);
	if (GWEN_MD_HashToBuffer("rmd160",
				 GWEN_Buffer_GetStart(msgBuf)+startPos,
				 endPos-startPos,
				 hbuf)) {
	  DBG_ERROR(AQHBCI_LOGDOMAIN,
		    "Could not rmd160 hash data");
	  AH_Job_SetStatus(j, AH_JobStatusError);
	  GWEN_Buffer_free(hbuf);
	  return AB_ERROR_INVALID;
	}
	else {
	  DBG_NOTICE(AQHBCI_LOGDOMAIN,
		   "RMD160 hashed job segment");
	  AH_Msg_SetItanHashBuffer(msg, hbuf);
	}
	break;
      }

      case 2: { /* SHA over buffer */
	GWEN_BUFFER *hbuf;

	hbuf=GWEN_Buffer_new(0, 32, 0, 1);
	if (GWEN_MD_HashToBuffer("sha1",
				 GWEN_Buffer_GetStart(msgBuf)+startPos,
				 endPos-startPos,
				 hbuf)) {
	  DBG_ERROR(AQHBCI_LOGDOMAIN,
		    "Could not sha1 hash data");
	  AH_Job_SetStatus(j, AH_JobStatusError);
	  GWEN_Buffer_free(hbuf);
	}
	else {
	  DBG_NOTICE(AQHBCI_LOGDOMAIN,
		   "SHA1 hashed job segment");
	  AH_Msg_SetItanHashBuffer(msg, hbuf);
	}
	break;
      }

      default: /* invalid mode */
	DBG_ERROR(AQHBCI_LOGDOMAIN,
		  "Invalid ITAN hash mode \"%d\"",
		  AH_Msg_GetItanHashMode(msg));
	AH_Job_SetStatus(j, AH_JobStatusError);
        return AB_ERROR_INVALID;
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
				 int timeout) {
  int rv;

  DBG_NOTICE(AQHBCI_LOGDOMAIN, "Sending queue");
  AB_Banking_ProgressLog(AH_HBCI_GetBankingApi(cbox->hbci),
			 0,
			 AB_Banking_LogLevelInfo,
                         I18N("Sending queue"));
  rv=AH_Dialog_SendMessage_Wait(dlg, msg, timeout);
  if (rv) {
    DBG_NOTICE(AQHBCI_LOGDOMAIN, "Could not send message");
    AB_Banking_ProgressLog(AH_HBCI_GetBankingApi(cbox->hbci),
			   0,
			   AB_Banking_LogLevelError,
			   I18N("Unable to send (network error)"));
    return rv;
  }
  DBG_NOTICE(AQHBCI_LOGDOMAIN, "Message sent");

  return 0;
}



int AH_Outbox__CBox_Itan(AH_OUTBOX__CBOX *cbox,
			 AH_DIALOG *dlg,
			 AH_JOBQUEUE *qJob,
			 int timeout){
  int rv;
  int process;

  process=AH_Dialog_GetItanProcessType(dlg);
  if (process==1)
    rv=AH_Outbox__CBox_Itan1(cbox, dlg, qJob, timeout);
  else if (process==2)
    rv=AH_Outbox__CBox_Itan2(cbox, dlg, qJob, timeout);
  else {
    DBG_ERROR(AQHBCI_LOGDOMAIN,
	      "iTAN method %d not supported", process);
    return AB_ERROR_INVALID;
  }

  return rv;
}



int AH_Outbox__CBox_SelectItanMode(AH_OUTBOX__CBOX *cbox,
				   AH_DIALOG *dlg) {
  AH_JOB *jTan;
  AB_USER *u;
  GWEN_DB_NODE *dbParams;
  GWEN_DB_NODE *dbMethod=0;
  GWEN_TYPE_UINT32 fid=0;
  int process=0;

  u=cbox->user;
  assert(u);

  /* test create HKTAN job */
  jTan=AH_Job_Tan_new(u, 0);
  if (!jTan) {
    DBG_WARN(AQHBCI_LOGDOMAIN, "Job HKTAN not (yet?) available");
    AH_Dialog_SetItanMethod(dlg, 0);
    AH_Dialog_SetItanProcessType(dlg, 0);
    return 0;
  }

  /* find DB_NODE for selected TanMethod */
  dbParams=AH_Job_GetParams(jTan);
  assert(dbParams);
  dbMethod=GWEN_DB_FindFirstGroup(dbParams, "tanMethod");
  while(dbMethod) {
    int fn;

    fid=0;
    fn=GWEN_DB_GetIntValue(dbMethod, "function", 0, 0);
    switch(fn) {
    case 999: fid=AH_USER_TANMETHOD_SINGLE_STEP; break;
    case 900: fid=AH_USER_TANMETHOD_TWO_STEP_00; break;
    case 990: fid=AH_USER_TANMETHOD_TWO_STEP_0; break;
    case 991: fid=AH_USER_TANMETHOD_TWO_STEP_1; break;
    case 992: fid=AH_USER_TANMETHOD_TWO_STEP_2; break;
    case 993: fid=AH_USER_TANMETHOD_TWO_STEP_3; break;
    case 994: fid=AH_USER_TANMETHOD_TWO_STEP_4; break;
    case 995: fid=AH_USER_TANMETHOD_TWO_STEP_5; break;
    case 996: fid=AH_USER_TANMETHOD_TWO_STEP_6; break;
    case 997: fid=AH_USER_TANMETHOD_TWO_STEP_7; break;
    default:
      DBG_ERROR(AQHBCI_LOGDOMAIN, "Invalid tan method %d", fn);
      fid=0;
    }

    if (fid) {
      GWEN_TYPE_UINT32 um;

      um=AH_User_GetTanMethods(u);
      if (
          (um==0) ||
          (um==AH_USER_TANMETHOD_SINGLE_STEP) ||
          (um & fid)) {
	process=GWEN_DB_GetIntValue(dbMethod, "process", 0, 0);
        if (process==1 || process==2) {
	  const char *s;
	  const char *lm=I18N_NOOP("Selecting iTAN mode \"%s\"");
	  char lbuf[256];

          s=GWEN_DB_GetCharValue(dbMethod, "methodName", 0, 0);
          if (!s || !*s)
            s=GWEN_DB_GetCharValue(dbMethod, "methodId", 0, 0);
          snprintf(lbuf, sizeof(lbuf), I18N(lm), s);
          lbuf[sizeof(lbuf)-1]=0;
          DBG_NOTICE(AQHBCI_LOGDOMAIN, "Selecting iTAN mode \"%s\"", s);
          AB_Banking_ProgressLog(AH_HBCI_GetBankingApi(cbox->hbci),
                                 0,
                                 AB_Banking_LogLevelInfo,
                                 lbuf);
          break;
        }
        else {
          DBG_NOTICE(AQHBCI_LOGDOMAIN,
                     "iTan process type \"%d\" not supported", process);
        }
      }
    }

    dbMethod=GWEN_DB_FindNextGroup(dbMethod, "tanMethod");
  }

  if (!dbMethod || !fid) {
    DBG_ERROR(AQHBCI_LOGDOMAIN,
	      "No matching iTAN mode found");
    AH_Job_free(jTan);
    return -1;
  }

  AH_Dialog_SetItanMethod(dlg, fid);
  AH_Dialog_SetItanProcessType(dlg, process);

  AH_Job_free(jTan);

  return 0;
}





