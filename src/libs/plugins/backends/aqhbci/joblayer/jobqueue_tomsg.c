/***************************************************************************
    begin       : Mon Mar 01 2004
    copyright   : (C) 2021 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "jobqueue_tomsg.h"

#include "aqhbci/banking/user_l.h"

#include "aqbanking/i18n_l.h"

#include <gwenhywfar/debug.h>
#include <gwenhywfar/gui.h>



/* ------------------------------------------------------------------------------------------------
 * forward declarations
 * ------------------------------------------------------------------------------------------------
 */


static int _messageSetupWithCryptoAndTan(AH_JOBQUEUE *jq, AH_DIALOG *dlg, AH_MSG *msg, const char *sTan);
static int _encodeJobs(AH_JOBQUEUE *jq, AH_MSG *msg);
static void _updateJobsAfterEncodingMessage(AH_JOBQUEUE *jq, AH_DIALOG *dlg, AH_MSG *msg);



/* ------------------------------------------------------------------------------------------------
 * implementations
 * ------------------------------------------------------------------------------------------------
 */



AH_MSG *AH_JobQueue_ToMessage(AH_JOBQUEUE *jq, AH_DIALOG *dlg)
{
  return AH_JobQueue_ToMessageWithTan(jq, dlg, NULL);
}



AH_MSG *AH_JobQueue_ToMessageWithTan(AH_JOBQUEUE *jq, AH_DIALOG *dlg, const char *sTan)
{
  AH_MSG *msg;
  int rv;

  assert(jq);
  assert(dlg);

  if (AH_JobQueue_GetCount(jq)<1) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "Empty queue");
    return NULL;
  }
  msg=AH_Msg_new(dlg);

  rv=_messageSetupWithCryptoAndTan(jq, dlg, msg, sTan);
  if (rv) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
    AH_Msg_free(msg);
    return NULL;
  }

  rv=_encodeJobs(jq, msg);
  if (rv<0) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
    AH_Msg_free(msg);
    return NULL;
  }

  rv=AH_Msg_EncodeMsg(msg);
  if (rv) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "Could not encode message (%d)", rv);
    AH_JobQueue_SetJobStatusOnMatch(jq, AH_JobStatusEncoded, AH_JobStatusError);
    AH_Msg_free(msg);
    return 0;
  }

  _updateJobsAfterEncodingMessage(jq, dlg, msg);

  DBG_DEBUG(AQHBCI_LOGDOMAIN, "Job queue encoded and ready to be sent");
  return msg;
}



int _messageSetupWithCryptoAndTan(AH_JOBQUEUE *jq, AH_DIALOG *dlg, AH_MSG *msg, const char *sTan)
{
  AB_USER *user;

  user=AH_JobQueue_GetUser(jq);

  AH_Msg_SetHbciVersion(msg, AH_User_GetHbciVersion(user));
  AH_Msg_SetSecurityProfile(msg, AH_JobQueue_GetSecProfile(jq));
  AH_Msg_SetSecurityClass(msg, AH_JobQueue_GetSecClass(jq));

  if (sTan && *sTan)
    AH_Msg_SetTan(msg, sTan);

  DBG_DEBUG(AQHBCI_LOGDOMAIN, "Adding queue to message (flags: %08x)", AH_JobQueue_GetFlags(jq));

  if (AH_JobQueue_GetFlags(jq) & AH_JOBQUEUE_FLAGS_NEEDTAN) {
    DBG_NOTICE(AQHBCI_LOGDOMAIN, "Queue needs a TAN");
  }
  else {
    DBG_NOTICE(AQHBCI_LOGDOMAIN, "Queue doesn't need a TAN");
  }
  AH_Msg_SetNeedTan(msg, (AH_JobQueue_GetFlags(jq) & AH_JOBQUEUE_FLAGS_NEEDTAN));
  AH_Msg_SetNoSysId(msg, (AH_JobQueue_GetFlags(jq) & AH_JOBQUEUE_FLAGS_NOSYSID));
  AH_Msg_SetSignSeqOne(msg, (AH_JobQueue_GetFlags(jq) & AH_JOBQUEUE_FLAGS_SIGNSEQONE));

  /* copy signers */
  if (AH_JobQueue_GetFlags(jq) & AH_JOBQUEUE_FLAGS_SIGN) {
    GWEN_STRINGLISTENTRY *se;

    se=GWEN_StringList_FirstEntry(AH_JobQueue_GetSigners(jq));
    if (!se) {
      DBG_ERROR(AQHBCI_LOGDOMAIN, "Signatures needed but no signer given");
      return GWEN_ERROR_GENERIC;
    }
    while (se) {
      DBG_NOTICE(AQHBCI_LOGDOMAIN, "Addign signer [%s]", GWEN_StringListEntry_Data(se));
      AH_Msg_AddSignerId(msg, GWEN_StringListEntry_Data(se));
      se=GWEN_StringListEntry_Next(se);
    } /* while */
  }

  /* copy crypter */
  if (AH_JobQueue_GetFlags(jq) & AH_JOBQUEUE_FLAGS_CRYPT) {
    const char *s;

    s=AH_User_GetPeerId(user);
    if (!s)
      s=AB_User_GetUserId(user);
    AH_Msg_SetCrypterId(msg, s);
  }

  return 0;
}



int _encodeJobs(AH_JOBQUEUE *jq, AH_MSG *msg)
{
  AH_JOB *j;
  unsigned int encodedJobs=0;

  j=AH_JobQueue_GetFirstJob(jq);
  while (j) {
    AH_JOB_STATUS st;

    st=AH_Job_GetStatus(j);
    /* only encode jobs which have not already been sent or which have no errors */
    if (st==AH_JobStatusEnqueued) {
      unsigned int firstSeg;
      unsigned int lastSeg;
      GWEN_DB_NODE *jargs;
      GWEN_XMLNODE *jnode;
      GWEN_BUFFER *msgBuf;

      DBG_DEBUG(AQHBCI_LOGDOMAIN, "Encoding job \"%s\"", AH_Job_GetName(j));
      jargs=AH_Job_GetArguments(j);
      jnode=AH_Job_GetXmlNode(j);
      if (strcasecmp(GWEN_XMLNode_GetData(jnode), "message")==0) {
        const char *s;

        s=GWEN_XMLNode_GetProperty(jnode, "name", 0);
        if (s) {
          DBG_NOTICE(AQHBCI_LOGDOMAIN, "Getting message specific data (%s)", s);
          jargs=GWEN_DB_GetGroup(jargs, GWEN_PATH_FLAGS_NAMEMUSTEXIST, s);
          if (!jargs) {
            DBG_NOTICE(AQHBCI_LOGDOMAIN, "No message specific data");
            jargs=AH_Job_GetArguments(j);
          }
        }
      }

      firstSeg=AH_Msg_GetCurrentSegmentNumber(msg);
      msgBuf=AH_Msg_GetBuffer(msg);
      assert(msgBuf);
      lastSeg=AH_Msg_AddNode(msg, jnode, jargs);
      if (!lastSeg) {
        DBG_INFO(AQHBCI_LOGDOMAIN, "Could not encode job \"%s\"", AH_Job_GetName(j));
        AH_Job_SetStatus(j, AH_JobStatusError);
      }
      else {
        AH_Job_SetFirstSegment(j, firstSeg);
        AH_Job_SetLastSegment(j, lastSeg);

        if (AH_Job_GetStatus(j)!=AH_JobStatusError) {
          DBG_DEBUG(AQHBCI_LOGDOMAIN, "Job \"%s\" encoded", AH_Job_GetName(j));
          AH_Job_SetStatus(j, AH_JobStatusEncoded);
          encodedJobs++;
        }
      }
    } /* if status matches */
    j=AH_Job_List_Next(j);
  } /* while */

  if (encodedJobs<1) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "No job in queue encoded");
    return GWEN_ERROR_GENERIC;
  }

  return 0;
}



void _updateJobsAfterEncodingMessage(AH_JOBQUEUE *jq, AH_DIALOG *dlg, AH_MSG *msg)
{
  AH_JOB *j;

  /*
   * inform all jobs that they have been encoded
   * this is needed for multi-message jobs so that they can prepare
   * themselves for the next message
   */
  j=AH_JobQueue_GetFirstJob(jq);
  AH_JobQueue_SetUsedTan(jq, AH_Msg_GetTan(msg));
  AH_JobQueue_SetUsedPin(jq, AH_Msg_GetPin(msg));
  while (j) {
    if (AH_Job_GetStatus(j)==AH_JobStatusEncoded) {
      const char *s;

      /* store some information about the message in the job */
      AH_Job_SetMsgNum(j, AH_Msg_GetMsgNum(msg));
      AH_Job_SetDialogId(j, AH_Dialog_GetDialogId(dlg));
      /* store expected signer and crypter (if any) */
      s=AH_Msg_GetExpectedSigner(msg);
      if (s)
        AH_Job_SetExpectedSigner(j, s);
      s=AH_Msg_GetExpectedCrypter(msg);
      if (s)
        AH_Job_SetExpectedCrypter(j, s);

      /* store used TAN (if any) */
      s=AH_Msg_GetTan(msg);
      if (s)
        AH_Job_SetUsedTan(j, s);
    }
    j=AH_Job_List_Next(j);
  } /* while */
}



