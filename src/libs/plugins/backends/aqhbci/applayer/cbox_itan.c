/***************************************************************************
    begin       : Mon Mar 01 2004
    copyright   : (C) 2019 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "aqhbci/applayer/cbox_itan.h"

#include "aqhbci/banking/provider_tan.h"
#include "aqhbci/applayer/cbox_itan1.h"
#include "aqhbci/applayer/cbox_itan2.h"

#include "aqbanking/i18n_l.h"

#include <gwenhywfar/mdigest.h>
#include <gwenhywfar/gui.h>

#include <ctype.h>



/* ------------------------------------------------------------------------------------------------
 * forward declarations
 * ------------------------------------------------------------------------------------------------
 */

static int _setupNeedTanAndSignersAndCrypter(AH_JOB *j, AH_MSG *msg, int doCopySigners);
static int _addJobNodesToMessage(AH_JOB *j, AH_MSG *msg);
static GWEN_DB_NODE *_getMessageSpecificArgsOrJobArgs(AH_JOB *j, const GWEN_XMLNODE *jnode);

static const AH_TAN_METHOD *_getAndCheckUserSelectedTanMethod(AB_USER *u, const AH_TAN_METHOD_LIST *tml);
static const AH_TAN_METHOD *_getAndCheckAutoSelectedTanMethod(AB_USER *u, const AH_TAN_METHOD_LIST *tml);


/* ------------------------------------------------------------------------------------------------
 * implementations
 * ------------------------------------------------------------------------------------------------
 */



int AH_OutboxCBox__Hash(int mode,
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



int AH_OutboxCBox_JobToMessage(AH_JOB *j, AH_MSG *msg, int doCopySigners)
{
  AB_USER *user;
  int rv;

  DBG_NOTICE(AQHBCI_LOGDOMAIN, "Encoding job \"%s\"", AH_Job_GetName(j));
  user=AH_Job_GetUser(j);
  assert(user);

  /* setup message */
  AH_Msg_SetHbciVersion(msg, AH_User_GetHbciVersion(user));

  rv=_setupNeedTanAndSignersAndCrypter(j, msg, doCopySigners);
  if (rv<0) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }

  rv=_addJobNodesToMessage(j, msg);
  if (rv<0) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }

  if (AH_Job_GetStatus(j)!=AH_JobStatusError) {
    DBG_NOTICE(AQHBCI_LOGDOMAIN, "Job \"%s\" encoded", AH_Job_GetName(j));
    AH_Job_SetStatus(j, AH_JobStatusEncoded);
  }

  return 0;
}



int _setupNeedTanAndSignersAndCrypter(AH_JOB *j, AH_MSG *msg, int doCopySigners)
{

  if (AH_Job_GetFlags(j) & AH_JOB_FLAGS_NEEDTAN) {
    DBG_NOTICE(AQHBCI_LOGDOMAIN, "Job needs a TAN");
  }
  else {
    DBG_NOTICE(AQHBCI_LOGDOMAIN, "Jobs doesn't need a TAN");
  }
  AH_Msg_SetNeedTan(msg, (AH_Job_GetFlags(j) & AH_JOB_FLAGS_NEEDTAN));

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
  return 0;
}



int _addJobNodesToMessage(AH_JOB *j, AH_MSG *msg)
{
  GWEN_DB_NODE *jargs;
  GWEN_XMLNODE *jnode;
  unsigned int firstSeg;
  unsigned int lastSeg;
  GWEN_BUFFER *msgBuf;
  uint32_t startPos;

  /* get arguments and XML node */
  jnode=AH_Job_GetXmlNode(j);
  jargs=_getMessageSpecificArgsOrJobArgs(j, jnode);

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

  AH_Job_SetFirstSegment(j, firstSeg);
  AH_Job_SetLastSegment(j, lastSeg);

  /* iTAN management */
  if (AH_Msg_GetItanHashBuffer(msg)==NULL) {
    int rv;
    uint32_t endPos;

    endPos=GWEN_Buffer_GetPos(msgBuf);
    rv=AH_OutboxCBox__Hash(AH_Msg_GetItanHashMode(msg),
			     (const uint8_t *)GWEN_Buffer_GetStart(msgBuf)+startPos,
			     endPos-startPos,
			     msg);
    if (rv) {
      DBG_ERROR(AQHBCI_LOGDOMAIN, "Could not hash data (%d)", rv);
      AH_Job_SetStatus(j, AH_JobStatusError);
      return rv;
    }
  }

  return 0;
}



GWEN_DB_NODE *_getMessageSpecificArgsOrJobArgs(AH_JOB *j, const GWEN_XMLNODE *jnode)
{
  GWEN_DB_NODE *jargs;

  jargs=AH_Job_GetArguments(j);

  if (strcasecmp(GWEN_XMLNode_GetData(jnode), "message")==0) {
    const char *s;

    s=GWEN_XMLNode_GetProperty(jnode, "name", 0);
    if (s) {
      DBG_NOTICE(AQHBCI_LOGDOMAIN, "Getting for message specific data (%s)", s);
      jargs=GWEN_DB_GetGroup(jargs, GWEN_PATH_FLAGS_NAMEMUSTEXIST, s);
      if (!jargs) {
        DBG_NOTICE(AQHBCI_LOGDOMAIN, "No message specific data");
        jargs=AH_Job_GetArguments(j);
      }
    }
  }

  return jargs;
}



int AH_OutboxCBox_SendAndReceiveQueueWithTan(AH_OUTBOX_CBOX *cbox,
                                               AH_DIALOG *dlg,
                                               AH_JOBQUEUE *qJob)
{
  int rv;
  int process;

  process=AH_Dialog_GetItanProcessType(dlg);
  if (process==1)
    rv=AH_OutboxCBox_Itan1(cbox, dlg, qJob);
  else if (process==2)
    rv=AH_OutboxCBox_SendAndReceiveQueueWithTan2(cbox, dlg, qJob);
  else {
    DBG_ERROR(AQHBCI_LOGDOMAIN,
              "iTAN method %d not supported", process);
    return GWEN_ERROR_INVALID;
  }

  return rv;
}



int AH_OutboxCBox_SelectItanMode(AH_OUTBOX_CBOX *cbox, AH_DIALOG *dlg)
{
  AB_USER *u;
  const AH_TAN_METHOD_LIST *tml;

  u=AH_OutboxCBox_GetUser(cbox);
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
    const AH_TAN_METHOD *tm;

    tm=_getAndCheckUserSelectedTanMethod(u, tml);
    if (tm==NULL)
      tm=_getAndCheckAutoSelectedTanMethod(u, tml);

    if (tm==NULL) {
      DBG_ERROR(AQHBCI_LOGDOMAIN, "No matching iTAN mode found");
      GWEN_Gui_ProgressLog(0, GWEN_LoggerLevel_Info, I18N("No valid iTAN method found"));
      return GWEN_ERROR_NOT_FOUND;
    }
    else {
      const char *s;

      s=AH_TanMethod_GetMethodName(tm);
      if (!s || !*s)
        s=AH_TanMethod_GetMethodId(tm);

      DBG_NOTICE(AQHBCI_LOGDOMAIN,
                 "Selecting TAN mode \"%s\" (%d, version %d, process %d))",
                 s,
                 AH_TanMethod_GetFunction(tm),
                 AH_TanMethod_GetGvVersion(tm),
                 AH_TanMethod_GetProcess(tm));
      GWEN_Gui_ProgressLog2(0,
                            GWEN_LoggerLevel_Info,
                            I18N("Selecting iTAN mode \"%s\" (%d, version %d, process %d)"),
                            s?s:I18N("(unnamed)"),
                            AH_TanMethod_GetFunction(tm),
                            AH_TanMethod_GetGvVersion(tm),
                            AH_TanMethod_GetProcess(tm));
      AH_Dialog_SetItanMethod(dlg, AH_TanMethod_GetFunction(tm));
      AH_Dialog_SetItanProcessType(dlg, AH_TanMethod_GetProcess(tm));
      AH_Dialog_SetTanJobVersion(dlg, AH_TanMethod_GetGvVersion(tm));
      AH_Dialog_SetTanMethodDescription(dlg, tm);
      return 0;
    }
  }
}



const AH_TAN_METHOD *_getAndCheckUserSelectedTanMethod(AB_USER *u, const AH_TAN_METHOD_LIST *tml)
{
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
	    DBG_INFO(AQHBCI_LOGDOMAIN, "Found description for selected TAN method %d (process: %d)", fn, proc);
	    break;
	  }
	  else {
	    DBG_NOTICE(AQHBCI_LOGDOMAIN, "iTan process type \"%d\" not supported, ignoring", proc);
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

  return tm;
}



const AH_TAN_METHOD *_getAndCheckAutoSelectedTanMethod(AB_USER *u, const AH_TAN_METHOD_LIST *tml)
{
  const AH_TAN_METHOD *tm;

  /* choose a method */
  DBG_INFO(AQHBCI_LOGDOMAIN, "Autoselecting a usable TAN method");

  tm=AH_TanMethod_List_First(tml);
  while (tm) {
    int proc;
    int fn;

    proc=AH_TanMethod_GetProcess(tm);
    fn=AH_TanMethod_GetFunction(tm);

    if (proc==1 || proc==2) {
      DBG_INFO(AQHBCI_LOGDOMAIN, "Found a possible description (fn=%d, process: %d)", fn, proc);
      if (AH_User_HasTanMethod(u, AH_TanMethod_GetFunction(tm))) {
	DBG_INFO(AQHBCI_LOGDOMAIN, "AH_User_HasTanMethod(%d): yes", fn);
	break;
      }
      else {
	DBG_INFO(AQHBCI_LOGDOMAIN, "AH_User_HasTanMethod(%d): no", fn);
      }
    }
    else {
      DBG_NOTICE(AQHBCI_LOGDOMAIN,
		 "iTan process type \"%d\" not supported, ignoring", proc);
    }

    tm=AH_TanMethod_List_Next(tm);
  }

  return tm;
}





void AH_OutboxCBox_CopyJobResultsToJobList(const AH_JOB *j, const AH_JOB_LIST *qjl)
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



int AH_OutboxCBox_InputTanWithChallenge(AH_OUTBOX_CBOX *cbox,
                                        AH_DIALOG *dialog,
                                        const char *sChallenge,
                                        const char *sChallengeHhd,
                                        char *passwordBuffer,
                                        int passwordMinLen,
                                        int passwordMaxLen)
{
  int rv;
  const AH_TAN_METHOD *tanMethodDescription=NULL;
  AB_PROVIDER *provider;
  AB_USER *user;

  provider=AH_OutboxCBox_GetProvider(cbox);
  user=AH_OutboxCBox_GetUser(cbox);

  tanMethodDescription=AH_Dialog_GetTanMethodDescription(dialog);
  assert(tanMethodDescription);

  rv=AH_Provider_InputTanWithChallenge(provider,
                                       user,
                                       tanMethodDescription,
                                       sChallenge, sChallengeHhd,
                                       passwordBuffer, passwordMinLen, passwordMaxLen);

  return rv;
}



