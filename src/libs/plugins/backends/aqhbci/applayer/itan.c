/***************************************************************************
    begin       : Mon Mar 01 2004
    copyright   : (C) 2019 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

/* This file is included by outbox.c */


#include "message_l.h"
#include "hhd_l.h"
#include "tanmechanism.h"

#include <gwenhywfar/mdigest.h>

#include <ctype.h>



/* forward declarations */
static int _addTextWithoutTags(const char *s, GWEN_BUFFER *obuf);
static void _addPhrasePleaseEnterTanForUser(AH_OUTBOX__CBOX *cbox, GWEN_BUFFER *bufGuiText);

static int _extractChallengeAndText(AH_OUTBOX__CBOX *cbox,
				    const char *sChallengeHhd,
				    const char *sChallenge,
				    GWEN_BUFFER *bufChallenge,
				    GWEN_BUFFER *bufGuiText);
static void _copyCompressedCodeIntoBuffer(const char *code, GWEN_BUFFER *cbuf);
static void _keepHhdBytes(GWEN_BUFFER *cbuf);




int AH_Outbox__CBox__Hash(int mode,
                          const uint8_t *p,
                          unsigned int l,
                          AH_MSG *msg)
{
  GWEN_MDIGEST *md=NULL;
  int rv;
  GWEN_BUFFER *hbuf;

  DBG_DEBUG(AQHBCI_LOGDOMAIN, "Hashmode: %d", mode);

  switch (mode) {
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
                          (const char *)GWEN_MDigest_GetDigestPtr(md),
                          GWEN_MDigest_GetDigestSize(md));
  GWEN_MDigest_free(md);
  AH_Msg_SetItanHashBuffer(msg, hbuf);

  DBG_NOTICE(AQHBCI_LOGDOMAIN, "Hashed job segment");

  return 0;
}



int AH_Outbox__CBox_JobToMessage(AH_JOB *j, AH_MSG *msg, int doCopySigners)
{
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

  if (doCopySigners) {
    /* copy signers */
    if (AH_Job_GetFlags(j) & AH_JOB_FLAGS_SIGN) {
      GWEN_STRINGLISTENTRY *se;

      se=GWEN_StringList_FirstEntry(AH_Job_GetSigners(j));
      if (!se) {
	DBG_ERROR(AQHBCI_LOGDOMAIN, "Signatures needed but no signer given");
	return GWEN_ERROR_INVALID;
      }
      while (se) {
	AH_Msg_AddSignerId(msg, GWEN_StringListEntry_Data(se));
	se=GWEN_StringListEntry_Next(se);
      } /* while */
    }
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
                               (const uint8_t *)GWEN_Buffer_GetStart(msgBuf)+startPos,
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



int AH_Outbox__CBox_SendAndReceiveQueueWithTan(AH_OUTBOX__CBOX *cbox,
                                               AH_DIALOG *dlg,
                                               AH_JOBQUEUE *qJob)
{
  int rv;
  int process;

  process=AH_Dialog_GetItanProcessType(dlg);
  if (process==1)
    rv=AH_Outbox__CBox_Itan1(cbox, dlg, qJob);
  else if (process==2)
    rv=AH_Outbox__CBox_SendAndReceiveQueueWithTan2(cbox, dlg, qJob);
  else {
    DBG_ERROR(AQHBCI_LOGDOMAIN,
              "iTAN method %d not supported", process);
    return GWEN_ERROR_INVALID;
  }

  return rv;
}



int AH_Outbox__CBox_SelectItanMode(AH_OUTBOX__CBOX *cbox,
                                   AH_DIALOG *dlg)
{
  AB_USER *u;
  const AH_TAN_METHOD_LIST *tml;

  u=cbox->user;
  assert(u);

  tml=AH_User_GetTanMethodDescriptions(u);
  if (tml==NULL || AH_TanMethod_List_GetCount(tml)<1) {
    /* no or empty list, select 999 */
    DBG_WARN(AQHBCI_LOGDOMAIN, "No tan methods, trying One-Step TAN");
    GWEN_Gui_ProgressLog(0,
                         GWEN_LoggerLevel_Warning,
                         I18N("There are no tan method descriptions (yet), trying One-Step TAN."));
    AH_Dialog_SetItanMethod(dlg, 999);
    AH_Dialog_SetItanProcessType(dlg, 1);
    AH_Dialog_SetTanJobVersion(dlg, 0);
    return 0;
  }
  else {
    const AH_TAN_METHOD *tm=NULL;
    int fn;

    fn=AH_User_GetSelectedTanMethod(u);
    if (fn) {
      int utFunction;
      int utJobVersion;

      utFunction=fn % 1000;
      utJobVersion=fn / 1000;

      DBG_INFO(AQHBCI_LOGDOMAIN, "Selected TAN method: %d (Job version %d, Function %d)", fn, utFunction, utJobVersion);
      if (AH_User_HasTanMethod(u, utFunction)) {
        tm=AH_TanMethod_List_First(tml);
        while (tm) {
          int proc;

          if (AH_TanMethod_GetFunction(tm)==utFunction && AH_TanMethod_GetGvVersion(tm)==utJobVersion) {
            proc=AH_TanMethod_GetProcess(tm);
            if (proc==1 || proc==2) {
              DBG_INFO(AQHBCI_LOGDOMAIN, "Found description for selected TAN method %d (process: %d)",
                       fn, proc);
              break;
            }
            else {
              DBG_NOTICE(AQHBCI_LOGDOMAIN,
                         "iTan process type \"%d\" not supported, ignoring", proc);
            }
          }

          tm=AH_TanMethod_List_Next(tm);
        }
        if (tm==NULL) {
          GWEN_Gui_ProgressLog2(0,
                                GWEN_LoggerLevel_Warning,
                                I18N("TAN method (%d) selected by user is no longer valid,"
                                     "please choose another one"),
                                fn);
        }
      }
      else {
        DBG_INFO(AQHBCI_LOGDOMAIN, "AH_User_HasTanMethod(%d): no", fn);
      }
    }
    else {
      DBG_INFO(AQHBCI_LOGDOMAIN, "No Tan method selected");
    }

    if (tm==NULL) {
      /* choose a method */
      DBG_INFO(AQHBCI_LOGDOMAIN, "Autoselecting a usable TAN method");

      tm=AH_TanMethod_List_First(tml);
      while (tm) {
        int proc;

        proc=AH_TanMethod_GetProcess(tm);
        if (proc==1 || proc==2) {
          DBG_INFO(AQHBCI_LOGDOMAIN, "Found description for selected TAN method %d (process: %d)",
                   fn, proc);
          if (AH_User_HasTanMethod(u, AH_TanMethod_GetFunction(tm))) {
            DBG_INFO(AQHBCI_LOGDOMAIN, "AH_User_HasTanMethod(%d): yes", AH_TanMethod_GetFunction(tm));
            break;
          }
          else {
            DBG_INFO(AQHBCI_LOGDOMAIN, "AH_User_HasTanMethod(%d): no", AH_TanMethod_GetFunction(tm));
          }
        }
        else {
          DBG_NOTICE(AQHBCI_LOGDOMAIN,
                     "iTan process type \"%d\" not supported, ignoring", proc);
        }

        tm=AH_TanMethod_List_Next(tm);
      }
    }

    if (tm==NULL) {
      DBG_ERROR(AQHBCI_LOGDOMAIN,
                "No matching iTAN mode found (fn=%d)", fn);
      GWEN_Gui_ProgressLog(0,
                           GWEN_LoggerLevel_Info,
                           I18N("No iTAN method available for automatic selection"));
      return GWEN_ERROR_NOT_FOUND;
    }
    else {
      const char *s;

      s=AH_TanMethod_GetMethodName(tm);
      if (!s || !*s)
        s=AH_TanMethod_GetMethodId(tm);

      DBG_NOTICE(AQHBCI_LOGDOMAIN, "Selecting iTAN mode \"%s\" (needs HKTAN:%d)", s, AH_TanMethod_GetGvVersion(tm));
      GWEN_Gui_ProgressLog2(0,
                            GWEN_LoggerLevel_Info,
                            I18N("Selecting iTAN mode \"%s\" (%d)"),
                            s?s:I18N("(unnamed)"),
                            AH_TanMethod_GetFunction(tm));
      AH_Dialog_SetItanMethod(dlg, AH_TanMethod_GetFunction(tm));
      AH_Dialog_SetItanProcessType(dlg, AH_TanMethod_GetProcess(tm));
      AH_Dialog_SetTanJobVersion(dlg, AH_TanMethod_GetGvVersion(tm));
      AH_Dialog_SetTanMethodDescription(dlg, tm);
      return 0;
    }
  }
}



void AH_Outbox__CBox_CopyJobResultsToJobList(const AH_JOB *j,
                                             const AH_JOB_LIST *qjl)
{
  /* dispatch results from jTan to all other members of the queue */
  if (qjl) {
    AH_RESULT_LIST *rl;

    /* segment results */
    rl=AH_Job_GetSegResults(j);
    if (rl) {
      AH_RESULT *or;

      or=AH_Result_List_First(rl);
      while (or) {
        AH_JOB *qj;

        qj=AH_Job_List_First(qjl);
        while (qj) {
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



int AH_Outbox__CBox_InputTanWithChallenge(AH_OUTBOX__CBOX *cbox,
					  AH_DIALOG *dialog,
					  const char *sChallenge,
					  const char *sChallengeHhd,
					  char *passwordBuffer,
					  int passwordMinLen,
					  int passwordMaxLen)
{
  int rv;
  GWEN_BUFFER *bufGuiText;
  GWEN_BUFFER *bufChallenge;
  const AH_TAN_METHOD *tanMethodDescription=NULL;
  AH_TAN_MECHANISM *tanMechanism;

  tanMethodDescription=AH_Dialog_GetTanMethodDescription(dialog);
  assert(tanMethodDescription);

  bufGuiText=GWEN_Buffer_new(0, 256, 0, 1);
  _addPhrasePleaseEnterTanForUser(cbox, bufGuiText);

  bufChallenge=GWEN_Buffer_new(0, 256, 0, 1);

  rv=_extractChallengeAndText(cbox, sChallengeHhd, sChallenge, bufChallenge, bufGuiText);
  if (rv<0) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
    GWEN_Buffer_free(bufChallenge);
    GWEN_Buffer_free(bufGuiText);
    return rv;
  }

  tanMechanism=AH_TanMechanism_Factory(tanMethodDescription);
  if (tanMechanism==NULL) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "Could not determine TAN mechanism to use");
    return rv;
  }

  rv=AH_TanMechanism_GetTan(tanMechanism,
			    cbox->user,
			    I18N("TAN Entry"),
			    GWEN_Buffer_GetStart(bufGuiText),
			    (const uint8_t*) GWEN_Buffer_GetStart(bufChallenge),
			    GWEN_Buffer_GetUsedBytes(bufChallenge),
			    passwordBuffer,
			    passwordMinLen,
			    passwordMaxLen);

  AH_TanMechanism_free(tanMechanism);

  GWEN_Buffer_free(bufChallenge);
  GWEN_Buffer_free(bufGuiText);

  return rv;
}



int _extractChallengeAndText(AH_OUTBOX__CBOX *cbox,
			     const char *sChallengeHhd,
			     const char *sChallenge,
			     GWEN_BUFFER *bufChallenge,
			     GWEN_BUFFER *bufGuiText)
{
  if (sChallengeHhd && *sChallengeHhd) {
    int rv;

    DBG_ERROR(AQHBCI_LOGDOMAIN, "ChallengeHHD is [%s]", sChallengeHhd);
    GWEN_Buffer_AppendString(bufChallenge, "0");
    /* use hex-encoded challenge */
    rv=GWEN_Text_FromHexBuffer(sChallengeHhd, bufChallenge);
    if (rv<0) {
      DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
      return rv;
    }
    DBG_ERROR(AQHBCI_LOGDOMAIN, "Created challenge HHUD is [%s]", GWEN_Buffer_GetStart(bufChallenge));

    /* get text */
    if (GWEN_Buffer_GetUsedBytes(bufGuiText)>0)
      GWEN_Buffer_AppendString(bufGuiText, "\n");
    if (sChallenge && *sChallenge)
      GWEN_Buffer_AppendString(bufGuiText, sChallenge);
    else
      GWEN_Buffer_AppendString(bufGuiText, I18N("Please enter the TAN from the device."));
    GWEN_Buffer_AppendString(bufGuiText, "\n");
  }
  else if (sChallenge && *sChallenge) {
    const char *s;

    /* look for "CHLGUC" */
    s=GWEN_Text_StrCaseStr(sChallenge, "CHLGUC");
    if (s) {
      GWEN_BUFFER *cbuf;

      /* extract challenge */
      DBG_ERROR(AQHBCI_LOGDOMAIN, "Challenge contains CHLGUC");
      cbuf=GWEN_Buffer_new(0, 256, 0, 1);
      _copyCompressedCodeIntoBuffer(sChallenge, cbuf);
      _keepHhdBytes(cbuf);

      GWEN_Buffer_AppendString(bufChallenge, GWEN_Buffer_GetStart(cbuf));
      GWEN_Buffer_free(cbuf);

      DBG_ERROR(AQHBCI_LOGDOMAIN, "Will use this challenge:");
      GWEN_Buffer_Dump(bufChallenge, 2);

      /* extract text */
      if (GWEN_Buffer_GetUsedBytes(bufGuiText)>0)
	GWEN_Buffer_AppendString(bufGuiText, "\n");

      s=GWEN_Text_StrCaseStr(sChallenge, "CHLGTEXT");
      if (s) {
        /* skip "CHLGTEXT" and 4 digits */
        s+=12;
        /* add rest of the message (replace HTML tags, if any) */
	_addTextWithoutTags(s, bufGuiText);
      }
      else {
        /* create own text */
        GWEN_Buffer_AppendString(bufGuiText, I18N("Please enter the TAN from the device."));
      }
    }
    else {
      /* no optical challenge */
      DBG_ERROR(AQHBCI_LOGDOMAIN, "Challenge contains no optical data");
      GWEN_Buffer_AppendString(bufGuiText, I18N("The server provided the following challenge:"));
      GWEN_Buffer_AppendString(bufGuiText, "\n");
      GWEN_Buffer_AppendString(bufGuiText, sChallenge);
    }
  }
  else {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "No challenge data given.");
    return GWEN_ERROR_BAD_DATA;
  }

  return 0;
}



void _copyCompressedCodeIntoBuffer(const char *code, GWEN_BUFFER *cbuf)
{
  const uint8_t *p;

  p=(const uint8_t*)code;
  while (*p) {
    uint8_t c;

    c=toupper(*p);
    if ((c>='0' && c<='9') || (c>='A' && c<='Z') || c==',')
      GWEN_Buffer_AppendByte(cbuf, c);
    p++;
  }
}



void _keepHhdBytes(GWEN_BUFFER *cbuf)
{
  const char *pStart=NULL;
  const char *pEnd=NULL;

  pStart=GWEN_Text_StrCaseStr(GWEN_Buffer_GetStart(cbuf), "CHLGUC");
  if (pStart) {
    pStart+=10; /* skip "CHLGUC" and following 4 digits */
    pEnd=GWEN_Text_StrCaseStr(pStart, "CHLGTEXT");
    if (pStart && pEnd) {
      GWEN_Buffer_Crop(cbuf, pStart-GWEN_Buffer_GetStart(cbuf), pEnd-pStart);
      GWEN_Buffer_SetPos(cbuf, 0);
      GWEN_Buffer_InsertByte(cbuf, '0');
      GWEN_Buffer_SetPos(cbuf, GWEN_Buffer_GetUsedBytes(cbuf));
    }
  }
}



void _addPhrasePleaseEnterTanForUser(AH_OUTBOX__CBOX *cbox, GWEN_BUFFER *bufGuiText)
{
  AB_BANKING *ab;
  AB_USER *user;
  char buffer[1024];
  const char *sUserName;
  const char *sBankName=NULL;
  AB_BANKINFO *bankInfo;

  user=cbox->user;
  assert(user);
  sUserName=AB_User_GetUserId(user);

  ab=AB_Provider_GetBanking(cbox->provider);
  assert(ab);

  /* find bank name */
  bankInfo=AB_Banking_GetBankInfo(ab, "de", "*", AB_User_GetBankCode(user));
  if (bankInfo)
    sBankName=AB_BankInfo_GetBankName(bankInfo);
  if (!sBankName)
    sBankName=AB_User_GetBankCode(user);

  snprintf(buffer, sizeof(buffer)-1,
	   I18N("Please enter the TAN for user %s at %s.\n"), sUserName, sBankName);
  buffer[sizeof(buffer)-1]=0;
  GWEN_Buffer_AppendString(bufGuiText, buffer);
  AB_BankInfo_free(bankInfo);
}



int _addTextWithoutTags(const char *s, GWEN_BUFFER *obuf)
{
  while (*s) {
    if (*s=='<') {
      const char *s2;
      int l;

      s2=s;
      s2++;
      while (*s2 && *s2!='>')
        s2++;
      l=s2-s-2;
      if (l>0) {
        const char *s3;

        s3=s;
        s3++;
        if (l==2) {
          if (strncasecmp(s3, "br", 2)==0)
            GWEN_Buffer_AppendString(obuf, "\n");
        }
        else if (l==3) {
          if (strncasecmp(s3, "br/", 3)==0)
            GWEN_Buffer_AppendString(obuf, "\n");
        }
      }
      s=s2; /* set s to position of closing bracket */
    }
    else
      GWEN_Buffer_AppendByte(obuf, *s);
    /* next char */
    s++;
  }

  return 0;
}




