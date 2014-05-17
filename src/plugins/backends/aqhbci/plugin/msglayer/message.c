/***************************************************************************
    begin       : Mon Mar 01 2004
    copyright   : (C) 2004-2010 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif


/* #define AH_MSG_HEAVY_DEBUG */

#include "message_p.h"
#include "aqhbci_l.h"
#include "hbci_l.h"
#include "dialog_l.h"
#include "msgengine_l.h"
#include "user_l.h"
#include <aqhbci/provider.h>

#include <gwenhywfar/gwenhywfar.h>
#include <gwenhywfar/debug.h>
#include <gwenhywfar/misc.h>
#include <gwenhywfar/text.h>
#include <gwenhywfar/list.h>
#include <gwenhywfar/mdigest.h>
#include <gwenhywfar/cryptkeysym.h>
#include <gwenhywfar/padd.h>
#include <gwenhywfar/gui.h>

#include <gwenhywfar/syncio_file.h>

#include <aqbanking/banking.h>
#include <aqbanking/banking_be.h>

#include <gwenhywfar/version.h>
#include <gwenhywfar/directory.h>

#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>



GWEN_LIST_FUNCTIONS(AH_MSG, AH_Msg);




/* --------------------------------------------------------------- FUNCTION */
int AH_Msg_AddSignerId(AH_MSG *hmsg, const char *s) {
  assert(hmsg);
  if (hmsg->nodes) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "Signers must be added before nodes !");
    return GWEN_ERROR_INVALID;
  }
  return GWEN_StringList_AppendString(hmsg->signerIdList, s, 0, 1);
}



/* --------------------------------------------------------------- FUNCTION */
unsigned int AH_Msg_GetSignerCount(AH_MSG *hmsg){
  assert(hmsg);
  return GWEN_StringList_Count(hmsg->signerIdList);
}



/* --------------------------------------------------------------- FUNCTION */
GWEN_BUFFER *AH_Msg_GetBuffer(AH_MSG *hmsg){
  assert(hmsg);
  return hmsg->buffer;
}



/* --------------------------------------------------------------- FUNCTION */
GWEN_BUFFER *AH_Msg_TakeBuffer(AH_MSG *hmsg){
  GWEN_BUFFER *bf;

  assert(hmsg);
  bf=hmsg->buffer;
  hmsg->buffer=0;
  return bf;
}



/* --------------------------------------------------------------- FUNCTION */
void AH_Msg_SetBuffer(AH_MSG *hmsg, GWEN_BUFFER *bf){
  assert(hmsg);
  GWEN_Buffer_free(hmsg->buffer);
  hmsg->buffer=bf;
}



/* --------------------------------------------------------------- FUNCTION */
unsigned int AH_Msg_GetMsgNum(const AH_MSG *hmsg){
  assert(hmsg);
  return hmsg->msgNum;
}



/* --------------------------------------------------------------- FUNCTION */
unsigned int AH_Msg_GetMsgRef(const AH_MSG *hmsg){
  assert(hmsg);
  return hmsg->refMsgNum;
}



/* --------------------------------------------------------------- FUNCTION */
void AH_Msg_SetMsgRef(AH_MSG *hmsg, unsigned int i){
  assert(hmsg);
  hmsg->refMsgNum=i;
}



/* --------------------------------------------------------------- FUNCTION */
unsigned int AH_Msg_GetNodes(AH_MSG *hmsg){
  assert(hmsg);
  return hmsg->nodes;
}



/* --------------------------------------------------------------- FUNCTION */
AH_MSG *AH_Msg_new(AH_DIALOG *dlg){
  AH_MSG *hmsg;

  assert(dlg);
  GWEN_NEW_OBJECT(AH_MSG, hmsg);
  GWEN_LIST_INIT(AH_MSG, hmsg);
  hmsg->dialog=dlg;
  AH_Dialog_Attach(dlg);
  hmsg->buffer=GWEN_Buffer_new(0, AH_MSG_DEFAULTSIZE, 0, 1);
  GWEN_Buffer_ReserveBytes(hmsg->buffer, AH_MSG_DEFAULTRESERVE);
  GWEN_Buffer_SetStep(hmsg->buffer, AH_MSG_DEFAULTSTEP);
  hmsg->signerIdList=GWEN_StringList_new();
  return hmsg;
}



/* --------------------------------------------------------------- FUNCTION */
void AH_Msg_free(AH_MSG *hmsg){
  if (hmsg) {
    DBG_DEBUG(AQHBCI_LOGDOMAIN, "Destroying AH_MSG");
    GWEN_LIST_FINI(AH_MSG, hmsg);
    GWEN_StringList_free(hmsg->signerIdList);
    GWEN_Buffer_free(hmsg->itanHashBuffer);
    GWEN_Buffer_free(hmsg->buffer);
    GWEN_Buffer_free(hmsg->origbuffer);
    AH_Dialog_free(hmsg->dialog);
    free(hmsg->crypterId);
    free(hmsg->resultText);
    free(hmsg->resultParam);
    free(hmsg->usedTan);
    free(hmsg->expectedSigner);
    free(hmsg->expectedCrypter);
    if (hmsg->usedPin) {
      memset(hmsg->usedPin, 0, strlen(hmsg->usedPin));
      free(hmsg->usedPin);
    }
    GWEN_DB_Group_free(hmsg->decodedMsg);

    GWEN_FREE_OBJECT(hmsg);
  }
}



/* --------------------------------------------------------------- FUNCTION */
int AH_Msg_GetSecurityProfile(const AH_MSG *hmsg) {
  assert(hmsg);
  return hmsg->secProfile;
}



/* --------------------------------------------------------------- FUNCTION */
void AH_Msg_SetSecurityProfile(AH_MSG *hmsg, int i) {
  assert(hmsg);
  hmsg->secProfile=i;
}



int AH_Msg_GetSecurityClass(const AH_MSG *hmsg) {
  assert(hmsg);
  return hmsg->secClass;
}



void AH_Msg_SetSecurityClass(AH_MSG *hmsg, int i) {
  assert(hmsg);
  hmsg->secClass=i;
}



/* --------------------------------------------------------------- FUNCTION */
int AH_Msg_AddMsgTail(AH_MSG *hmsg){
  GWEN_XMLNODE *node;
  GWEN_DB_NODE *cfg;
  int rv;
  GWEN_MSGENGINE *e;

  assert(hmsg);
  e=AH_Dialog_GetMsgEngine(hmsg->dialog);
  assert(e);

  node=GWEN_MsgEngine_FindNodeByPropertyStrictProto(e,
						    "SEG",
						    "id",
						    0,
						    "MsgTail");
  if (!node) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "Segment \"MsgTail\" not found");
    return -1;
  }

  cfg=GWEN_DB_Group_new("msgtail");
  GWEN_DB_SetIntValue(cfg, GWEN_DB_FLAGS_DEFAULT,
                      "head/seq",
                      hmsg->lastSegment+1);
  GWEN_Buffer_SetPos(hmsg->buffer, GWEN_Buffer_GetUsedBytes(hmsg->buffer));
  rv=GWEN_MsgEngine_CreateMessageFromNode(e,
                                          node,
                                          hmsg->buffer,
                                          cfg);
  GWEN_DB_Group_free(cfg);
  if (rv) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "Could not create msgTail");
    return -1;
  }

  return 0;
}



/* --------------------------------------------------------------- FUNCTION */
int AH_Msg_AddMsgHead(AH_MSG *hmsg) {
  GWEN_XMLNODE *node;
  GWEN_DB_NODE *cfg;
  GWEN_BUFFER *hbuf;
  unsigned int msize;
  int rv;
  GWEN_MSGENGINE *e;

  assert(hmsg);
  e=AH_Dialog_GetMsgEngine(hmsg->dialog);
  assert(e);

  node=GWEN_MsgEngine_FindNodeByPropertyStrictProto(e,
						    "SEG",
						    "id",
						    0,
						    "MsgHead");
  if (!node) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "Segment \"MsgHead\"not found");
    return -1;
  }

  cfg=GWEN_DB_Group_new("msghead");
  GWEN_DB_SetCharValue(cfg, GWEN_DB_FLAGS_DEFAULT,
                       "dialogid",
                       AH_Dialog_GetDialogId(hmsg->dialog));
  GWEN_DB_SetIntValue(cfg, GWEN_DB_FLAGS_DEFAULT,
                      "msgnum",
                      hmsg->msgNum);
  if (hmsg->refMsgNum) {
    DBG_DEBUG(AQHBCI_LOGDOMAIN, "Adding Reference Message Number");
    /* add message reference */
    GWEN_DB_SetIntValue(cfg, GWEN_DB_FLAGS_DEFAULT,
                        "msgref/msgnum", hmsg->refMsgNum);
    GWEN_DB_SetCharValue(cfg, GWEN_DB_FLAGS_DEFAULT,
                         "msgref/dialogid",
                         AH_Dialog_GetDialogId(hmsg->dialog));
  }
  DBG_DEBUG(AQHBCI_LOGDOMAIN, "Protocol version is %d",
           GWEN_MsgEngine_GetProtocolVersion(e));

  GWEN_DB_SetIntValue(cfg, GWEN_DB_FLAGS_DEFAULT,
                      "hversion",
                      GWEN_MsgEngine_GetProtocolVersion(e));
  GWEN_DB_SetIntValue(cfg, GWEN_DB_FLAGS_DEFAULT,
                      "size", 1);
  GWEN_DB_SetIntValue(cfg, GWEN_DB_FLAGS_DEFAULT,
                      "head/seq", 1);
  hbuf=GWEN_Buffer_new(0, 128, 0, 1);

  /* create first version of msgHead just to calculate the size */
  rv=GWEN_MsgEngine_CreateMessageFromNode(e,
                                          node,
                                          hbuf,
                                          cfg);
  if (rv) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "Could not create msgHead");
    GWEN_Buffer_free(hbuf);
    GWEN_DB_Group_free(cfg);
    return -1;
  }

  msize=GWEN_Buffer_GetUsedBytes(hmsg->buffer)+
    GWEN_Buffer_GetUsedBytes(hbuf);
  DBG_DEBUG(AQHBCI_LOGDOMAIN, "Message size is: %d", msize);
  GWEN_DB_SetIntValue(cfg,
                      GWEN_DB_FLAGS_OVERWRITE_VARS,
                      "size", msize);
  GWEN_DB_SetIntValue(cfg, GWEN_DB_FLAGS_DEFAULT,
                      "head/seq", 1);
  GWEN_Buffer_Reset(hbuf);

  /* create final version of msgHead (we now know the size) */
  rv=GWEN_MsgEngine_CreateMessageFromNode(e,
                                          node,
                                          hbuf,
                                          cfg);
  GWEN_DB_Group_free(cfg);
  if (rv) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "Could not create 2nd version of msgHead");
    GWEN_Buffer_free(hbuf);
    return -1;
  }

  /* insert msgHead */
  GWEN_Buffer_SetPos(hmsg->buffer, 0);
  if (GWEN_Buffer_InsertBuffer(hmsg->buffer, hbuf)) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "Could not insert msgHead");
    GWEN_Buffer_free(hbuf);
    return -1;
  }

  GWEN_Buffer_free(hbuf);
  return 0;
}



/* --------------------------------------------------------------- FUNCTION */
unsigned int AH_Msg_GetCurrentSegmentNumber(AH_MSG *hmsg) {
  if (hmsg->firstSegment==0) {
    unsigned int rv;

    rv=2;
    if (hmsg->enableInsert)
      rv++;
    rv+=GWEN_StringList_Count(hmsg->signerIdList);
    return rv;
  }
  return hmsg->lastSegment+1;
}



/* --------------------------------------------------------------- FUNCTION */
unsigned int AH_Msg_AddNode(AH_MSG *hmsg,
                            GWEN_XMLNODE *node,
                            GWEN_DB_NODE *data) {
  int rv;
  GWEN_MSGENGINE *e;
  unsigned int usedBefore;

  assert(hmsg);
  assert(node);
  assert(data);

  e=AH_Dialog_GetMsgEngine(hmsg->dialog);
  assert(e);

  if (hmsg->firstSegment==0) {
    /* first node */
    hmsg->firstSegment=2;
    if (hmsg->enableInsert)
      hmsg->firstSegment++;
    hmsg->firstSegment+=GWEN_StringList_Count(hmsg->signerIdList);
    GWEN_MsgEngine_SetIntValue(e,
                               "SegmentNumber",
                               hmsg->firstSegment);
    hmsg->lastSegment=hmsg->firstSegment-1;
  }

  usedBefore=GWEN_Buffer_GetUsedBytes(hmsg->buffer);
  rv=GWEN_MsgEngine_CreateMessageFromNode(e,
                                          node,
                                          hmsg->buffer,
                                          data);
  if (rv) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
    GWEN_Buffer_Crop(hmsg->buffer, 0, usedBefore);
    GWEN_Buffer_SetPos(hmsg->buffer, usedBefore);

    DBG_ERROR(AQHBCI_LOGDOMAIN, "Buffer:");
    GWEN_Buffer_Dump(hmsg->buffer, 2);
    DBG_ERROR(AQHBCI_LOGDOMAIN, "XML:");
    GWEN_XMLNode_Dump(node, 2);
    DBG_ERROR(0, "MsgEngine - mode: %s version:%d",
              GWEN_MsgEngine_GetMode(e),
              GWEN_MsgEngine_GetProtocolVersion(e));
    return 0;
  }
  hmsg->lastSegment=GWEN_MsgEngine_GetIntValue(e,
                                               "SegmentNumber",
                                               1)-1;
  hmsg->nodes++;
  return hmsg->lastSegment;
}



/* --------------------------------------------------------------- FUNCTION */
unsigned int AH_Msg_InsertNode(AH_MSG *hmsg,
                               GWEN_XMLNODE *node,
                               GWEN_DB_NODE *data) {
  int rv;
  GWEN_MSGENGINE *e;
  GWEN_BUFFER *tmpbuf;
  unsigned int pos;

  assert(hmsg);
  assert(node);
  assert(data);

  if (hmsg->nodes==0)
    return AH_Msg_AddNode(hmsg, node, data);

  e=AH_Dialog_GetMsgEngine(hmsg->dialog);
  assert(e);

  if (!hmsg->enableInsert) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "Inserting a node without enableInsert flag");
    return 0;
  }
  if (hmsg->firstSegment<(2+GWEN_StringList_Count(hmsg->signerIdList))) {
    DBG_ERROR(AQHBCI_LOGDOMAIN,
              "First segment is %d (%d signers), can't insert a new one",
              hmsg->firstSegment, GWEN_StringList_Count(hmsg->signerIdList));
    return 0;
  }

  hmsg->firstSegment--;

  GWEN_MsgEngine_SetIntValue(e,
                             "SegmentNumber",
                             hmsg->firstSegment);

  tmpbuf=GWEN_Buffer_new(0, 512, 0, 1);
  rv=GWEN_MsgEngine_CreateMessageFromNode(e,
                                          node,
                                          tmpbuf,
                                          data);
  GWEN_MsgEngine_SetIntValue(e,
                             "SegmentNumber",
                             hmsg->lastSegment+1);
  if (rv) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "here");
    GWEN_Buffer_free(tmpbuf);
    return 0;
  }

  pos=GWEN_Buffer_GetPos(hmsg->buffer);
  GWEN_Buffer_SetPos(hmsg->buffer, 0);
  GWEN_Buffer_InsertBuffer(hmsg->buffer, tmpbuf);
  GWEN_Buffer_SetPos(hmsg->buffer, pos+GWEN_Buffer_GetUsedBytes(tmpbuf));
  GWEN_Buffer_free(tmpbuf);

  hmsg->nodes++;
  return hmsg->firstSegment;
}



/* --------------------------------------------------------------- FUNCTION */
int AH_Msg_EncodeMsg(AH_MSG *hmsg) {
  GWEN_MSGENGINE *e;
  int rv;

  assert(hmsg);

  e=AH_Dialog_GetMsgEngine(hmsg->dialog);
  assert(e);
  GWEN_MsgEngine_SetProtocolVersion(e, hmsg->hbciVersion);

  if (hmsg->firstSegment==0) {
    /* first node */
    hmsg->firstSegment=2;
    hmsg->firstSegment+=GWEN_StringList_Count(hmsg->signerIdList);
    GWEN_MsgEngine_SetIntValue(e,
                               "SegmentNumber",
                               hmsg->firstSegment);
    hmsg->lastSegment=hmsg->firstSegment-1;
  }

  hmsg->msgNum=AH_Dialog_GetNextMsgNum(hmsg->dialog);
  GWEN_MsgEngine_SetIntValue(AH_Dialog_GetMsgEngine(hmsg->dialog),
                             "MessageNumber",
                             hmsg->msgNum);

  GWEN_MsgEngine_SetValue(AH_Dialog_GetMsgEngine(hmsg->dialog),
                          "DialogId",
                          AH_Dialog_GetDialogId(hmsg->dialog));

  /* sign message */
  DBG_DEBUG(AQHBCI_LOGDOMAIN, "Letting all signers sign");
  if (GWEN_StringList_Count(hmsg->signerIdList)) {
    GWEN_BUFFER *rawBuf;
    GWEN_STRINGLISTENTRY *se;

    rawBuf=GWEN_Buffer_dup(hmsg->buffer);
    se=GWEN_StringList_FirstEntry(hmsg->signerIdList);
    while (se) {
      rv=AH_Msg__Sign(hmsg, rawBuf, GWEN_StringListEntry_Data(se));
      if (rv) {
        GWEN_Buffer_free(rawBuf);
        DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
        return rv;
      }
      se=GWEN_StringListEntry_Next(se);
    } /* while */
    GWEN_Buffer_free(rawBuf);
  } /* if signing is needed */
  DBG_DEBUG(AQHBCI_LOGDOMAIN, "Letting all signers sign: done");

  /* log unencrypted message */
  AH_Msg_LogMessage(hmsg, hmsg->buffer, 0, 0);

  /* encrypt message */
  if (hmsg->crypterId) {
    DBG_DEBUG(AQHBCI_LOGDOMAIN, "Encrypting message");
    rv=AH_Msg__Encrypt(hmsg);
    if (rv) {
      DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
      return rv;
    }
    DBG_DEBUG(AQHBCI_LOGDOMAIN, "Encrypting message: done");
  }

  /* add msg tail */
  DBG_DEBUG(AQHBCI_LOGDOMAIN, "Adding message tail");
  if (AH_Msg_AddMsgTail(hmsg)) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "here");
    return -1;
  }
  DBG_DEBUG(AQHBCI_LOGDOMAIN, "Adding message tail: done");

  /* add msg head */
  DBG_DEBUG(AQHBCI_LOGDOMAIN, "Adding message head");
  if (AH_Msg_AddMsgHead(hmsg)) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "here");
    return -1;
  }
  DBG_DEBUG(AQHBCI_LOGDOMAIN, "Adding message head: done");

  /* log final message */
  AH_Msg_LogMessage(hmsg, hmsg->buffer, 0, 1);

  DBG_DEBUG(AQHBCI_LOGDOMAIN, "Message finished");
  return 0;
}














/* --------------------------------------------------------------- FUNCTION */
/* return -1 on error (with group "seg/error" set) or -2 if the message is
 * faulty */
int AH_Msg_ReadSegment(AH_MSG *msg,
                       GWEN_MSGENGINE *e,
                       const char *gtype,
                       GWEN_BUFFER *mbuf,
                       GWEN_DB_NODE *gr,
                       unsigned int flags) {
  GWEN_XMLNODE *node;
  unsigned int posBak;
  const char *p;
  GWEN_DB_NODE *tmpdb;
  int segVer;

  /* find head segment description */
  tmpdb=GWEN_DB_Group_new("head");
  node=GWEN_MsgEngine_FindGroupByProperty(e,
                                          "id",
                                          0,
                                          "SegHead");
  if (node==0) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "Segment description not found (internal error)");
    GWEN_DB_Group_free(tmpdb);
    return -2;
  }

  /* parse head segment */
  posBak=GWEN_Buffer_GetPos(mbuf);
  if (GWEN_MsgEngine_ParseMessage(e,
                                  node,
                                  mbuf,
                                  tmpdb,
                                  flags)) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "Error parsing segment head");
    GWEN_DB_Group_free(tmpdb);
    return -2;
  }

  GWEN_Buffer_SetPos(mbuf, posBak);

  /* get segment code */
  segVer=GWEN_DB_GetIntValue(tmpdb,
                             "version",
                             0,
                             0);
  p=GWEN_DB_GetCharValue(tmpdb,
                         "code",
                         0,
                         0);
  if (!p) {
    DBG_ERROR(AQHBCI_LOGDOMAIN,
              "No segment code for %s ? This seems to be a bad msg...",
              gtype);
    DBG_ERROR(AQHBCI_LOGDOMAIN, "Full message (pos=%04x)", posBak);
    GWEN_Text_DumpString(GWEN_Buffer_GetStart(mbuf),
                         GWEN_Buffer_GetUsedBytes(mbuf),
                         1);
    GWEN_DB_Dump(tmpdb, 1);
    GWEN_DB_Group_free(tmpdb);
    return -1;
  }

  /* try to find corresponding XML node */
  node=GWEN_MsgEngine_FindNodeByProperty(e,
                                         gtype,
                                         "code",
                                         segVer,
                                         p);
  if (node==0) {
    GWEN_DB_NODE *storegrp;
    unsigned int startPos;

    GWEN_Buffer_SetPos(mbuf, posBak);
    startPos=posBak;

    storegrp=GWEN_DB_GetGroup(gr,
                              GWEN_PATH_FLAGS_CREATE_GROUP,
                              p);
    assert(storegrp);
    GWEN_DB_AddGroup(storegrp, GWEN_DB_Group_dup(tmpdb));

    /* store the start position of this segment within the DB */
    GWEN_DB_SetIntValue(storegrp,
                        GWEN_DB_FLAGS_OVERWRITE_VARS,
                        "segment/pos",
                        startPos);
    GWEN_DB_SetIntValue(storegrp,
                        GWEN_DB_FLAGS_OVERWRITE_VARS,
                        "segment/error/code",
                        9130);
    GWEN_DB_SetCharValue(storegrp,
                         GWEN_DB_FLAGS_OVERWRITE_VARS,
                         "segment/error/text",
                         "Unbekanntes segment (Parser)");
    GWEN_DB_SetIntValue(storegrp,
                        GWEN_DB_FLAGS_OVERWRITE_VARS,
                        "segment/error/pos",
                        startPos);

    /* node not found, skip it */
    DBG_WARN(AQHBCI_LOGDOMAIN,
             "Unknown segment \"%s\" (Segnum=%d, version=%d, ref=%d)",
             p,
             GWEN_DB_GetIntValue(tmpdb, "seq", 0, -1),
             GWEN_DB_GetIntValue(tmpdb, "version", 0, -1),
             GWEN_DB_GetIntValue(tmpdb, "ref", 0, -1));
    if (GWEN_MsgEngine_SkipSegment(e, mbuf, '?', '\'')) {
      DBG_ERROR(AQHBCI_LOGDOMAIN, "Error skipping segment \"%s\"", p);
      GWEN_DB_Group_free(tmpdb);
      return -1;
    }
    /* store segment size within DB */
    GWEN_DB_SetIntValue(storegrp,
                        GWEN_DB_FLAGS_OVERWRITE_VARS,
                        "segment/length",
                        GWEN_Buffer_GetPos(mbuf)-startPos);
    /* handle trust info */
    if (flags & GWEN_MSGENGINE_READ_FLAGS_TRUSTINFO) {
      unsigned int usize;

      usize=GWEN_Buffer_GetPos(mbuf)-(startPos+1)-1;
      if (GWEN_MsgEngine_AddTrustInfo(e,
                                      GWEN_Buffer_GetStart(mbuf)+startPos,
                                      usize,
                                      p,
                                      GWEN_MsgEngineTrustLevelHigh,
                                      startPos)) {
        DBG_INFO(AQHBCI_LOGDOMAIN, "called from here");
        GWEN_DB_Group_free(tmpdb);
        return -1;
      }
    } /* if trustInfo handling wanted */
  }
  else {
    /* ok, node available, get the corresponding description and parse
     * the segment */
    const char *id;
    GWEN_DB_NODE *storegrp;
    unsigned int startPos;

    /* restore start position, since the segment head is part of a full
     * description, so we need to restart reading from the very begin */
    GWEN_Buffer_SetPos(mbuf, posBak);

    /* create group in DB for this segment */
    id=GWEN_XMLNode_GetProperty(node, "id", p);
    storegrp=GWEN_DB_GetGroup(gr,
                              GWEN_PATH_FLAGS_CREATE_GROUP,
                              id);
    assert(storegrp);

    /* store the start position of this segment within the DB */
    startPos=GWEN_Buffer_GetPos(mbuf);
    GWEN_DB_SetIntValue(storegrp,
                        GWEN_DB_FLAGS_OVERWRITE_VARS,
                        "segment/pos",
                        startPos);

    /* parse the segment */
    if (GWEN_MsgEngine_ParseMessage(e,
                                    node,
                                    mbuf,
                                    storegrp,
                                    flags)) {
      GWEN_DB_SetIntValue(storegrp,
                          GWEN_DB_FLAGS_OVERWRITE_VARS,
                          "segment/error/code",
                          9130);
      GWEN_DB_SetCharValue(storegrp,
                           GWEN_DB_FLAGS_OVERWRITE_VARS,
                           "segment/error/text",
                           "Syntaxfehler");
      GWEN_DB_SetIntValue(storegrp,
                          GWEN_DB_FLAGS_OVERWRITE_VARS,
                          "segment/error/pos",
                          GWEN_Buffer_GetPos(mbuf)-startPos);

      DBG_ERROR(AQHBCI_LOGDOMAIN, "Error parsing segment \"%s\"",p);
      GWEN_Text_DumpString(GWEN_Buffer_GetStart(mbuf)+startPos,
                           GWEN_Buffer_GetUsedBytes(mbuf)-startPos,
                           1);
      GWEN_DB_Group_free(tmpdb);
      return -1;
    }

    /* store segment size within DB */
    GWEN_DB_SetIntValue(storegrp,
                        GWEN_DB_FLAGS_OVERWRITE_VARS,
                        "segment/length",
                        GWEN_Buffer_GetPos(mbuf)-startPos);
    if (strcasecmp(id, "MsgHead")==0) {
      int protocolVersion;

      protocolVersion=GWEN_DB_GetIntValue(storegrp,
                                          "hversion",
                                          0, 0);
      if (!protocolVersion) {
        DBG_WARN(AQHBCI_LOGDOMAIN, "Unknown protocol version, using default of 210");
        protocolVersion=210;
      }
      /* set HBCI protocol version to be used (taken from MsgHead) */
      DBG_DEBUG(AQHBCI_LOGDOMAIN, "Setting protocol version to %d", protocolVersion);
      GWEN_MsgEngine_SetProtocolVersion(e, protocolVersion);
      msg->hbciVersion=protocolVersion;
    }
  } /* if node found */
  GWEN_DB_Group_free(tmpdb);

  return 0;
}



/* --------------------------------------------------------------- FUNCTION */
int AH_Msg_ReadMessage(AH_MSG *msg,
                       GWEN_MSGENGINE *e,
                       const char *gtype,
                       GWEN_BUFFER *mbuf,
                       GWEN_DB_NODE *gr,
                       unsigned int flags) {
  unsigned int segments;
  unsigned int errors;
  int rv;

  segments=0;
  errors=0;

  while(GWEN_Buffer_GetBytesLeft(mbuf)) {
    rv=AH_Msg_ReadSegment(msg, e, gtype, mbuf, gr, flags);
    if (rv==-2) {
      DBG_INFO(AQHBCI_LOGDOMAIN, "here");
      return -1;
    }
    else if (rv==-1) {
      DBG_INFO(AQHBCI_LOGDOMAIN, "here");

      DBG_ERROR(AQHBCI_LOGDOMAIN, "Error here:");
      GWEN_Buffer_Dump(mbuf, 2);
#ifdef AH_MSG_HEAVY_DEBUG
      return -1;
#endif
      if (GWEN_MsgEngine_SkipSegment(e, mbuf, '?', '\'')) {
        DBG_ERROR(AQHBCI_LOGDOMAIN, "Error skipping segment");
        return -1;
      }
      errors++;
    }
    segments++;
  } /* while */

  DBG_DEBUG(AQHBCI_LOGDOMAIN, "Parsed %d segments (%d had errors)",
           segments, errors);
  return 0;
}



/* --------------------------------------------------------------- FUNCTION */
int AH_Msg_SequenceCheck(GWEN_DB_NODE *gr) {
  GWEN_DB_NODE *n;
  unsigned int sn;
  unsigned int errors;

  DBG_DEBUG(AQHBCI_LOGDOMAIN, "Sequence check");

  sn=1;
  errors=0;
  n=GWEN_DB_GetFirstGroup(gr);
  while(n) {
    unsigned int rsn;

    rsn=GWEN_DB_GetIntValue(n, "head/seq", 0, 0);
    if (rsn<900) {
      if (rsn!=sn) {
        DBG_ERROR(AQHBCI_LOGDOMAIN,
                  "Unexpected sequence number (%d, expected %d)",
                  rsn, sn);
        GWEN_DB_Dump(n, 2);

        GWEN_DB_SetIntValue(n,
                            GWEN_DB_FLAGS_OVERWRITE_VARS,
                            "segment/error/code",
                            9120);
        GWEN_DB_SetCharValue(n,
                             GWEN_DB_FLAGS_OVERWRITE_VARS,
                             "segment/error/text",
                             "Unerwartete Segmentnummer");
        errors++;
      }
      sn++;
    }
    n=GWEN_DB_GetNextGroup(n);
  } /* while */

  if (errors)
    return -1;
  DBG_DEBUG(AQHBCI_LOGDOMAIN, "Sequence check ok");
  return 0;
}



/* --------------------------------------------------------------- FUNCTION */
int AH_Msg_DecodeMsg(AH_MSG *hmsg,
                     GWEN_DB_NODE *gr,
                     unsigned int flags) {
  GWEN_MSGENGINE *e;
  int rv;
  GWEN_DB_NODE *n, *n2;
  AB_USER *u;
  const char *mode;
  uint32_t expMsgNum;
  uint32_t guiid;

  assert(hmsg->dialog);
  e=AH_Dialog_GetMsgEngine(hmsg->dialog);
  assert(e);

  /* set mode */
  u=AH_Dialog_GetDialogOwner(hmsg->dialog);
  assert(u);

  guiid=0;

  mode=AH_CryptMode_toString(AH_User_GetCryptMode(u));
  DBG_NOTICE(AQHBCI_LOGDOMAIN, "Mode is: %s", mode);
  GWEN_MsgEngine_SetMode(e, mode);

  DBG_NOTICE(AQHBCI_LOGDOMAIN, "Decoding message:");

  /* log encrypted message */
  AH_Msg_LogMessage(hmsg, hmsg->buffer, 1, 1);

  GWEN_Buffer_Rewind(hmsg->buffer);
  rv=AH_Msg_ReadMessage(hmsg, e, "SEG", hmsg->buffer, gr, flags);
  if (rv) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "here");
    return GWEN_ERROR_BAD_DATA;
  }

  /* take some values out of the message head (with checks) */
  n=GWEN_DB_GetGroup(gr,
                     GWEN_DB_FLAGS_DEFAULT |
                     GWEN_PATH_FLAGS_NAMEMUSTEXIST,
                     "MsgHead");
  if (!n) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "No message head segment");
    return GWEN_ERROR_BAD_DATA;
  }

  /* sample message number */
  expMsgNum=AH_Dialog_GetLastMsgNum(hmsg->dialog);
  hmsg->msgNum=GWEN_DB_GetIntValue(n, "msgnum", 0, 0);
  if (AH_Dialog_CheckReceivedMsgNum(hmsg->dialog, hmsg->msgNum)) {
    DBG_WARN(AQHBCI_LOGDOMAIN, "Bad message number, ignoring");
    GWEN_Gui_ProgressLog(0,
			 GWEN_LoggerLevel_Warning,
			 I18N("Bad message number, ignoring"));
    /* FIXME: this is sometimes returned by the Sparkasse server, when
     * the server is currently out of service
     * We will for now ignore this, since this message still returns an
     * error code and text which might be of interest to the user.
     */
    /* return -1; */
    hmsg->msgNum=expMsgNum;
  }

  /* sample dialog id if necessary */
  if (strcasecmp(AH_Dialog_GetDialogId(hmsg->dialog), "0")==0) {
    /* dialog id not yet known, copy it */
    const char *p;

    p=GWEN_DB_GetCharValue(n, "dialogid", 0, 0);
    if (!p) {
      DBG_ERROR(AQHBCI_LOGDOMAIN, "No dialog id in message");
      return GWEN_ERROR_BAD_DATA;
    }
    if (AH_Dialog_GetFlags(hmsg->dialog) & AH_DIALOG_FLAGS_INITIATOR) {
      DBG_DEBUG(AQHBCI_LOGDOMAIN, "Setting new dialog id (%s)", p);
      AH_Dialog_SetDialogId(hmsg->dialog, p);
      GWEN_MsgEngine_SetValue(e, "dialogid", p);
    }
    else {
      const char *myDlgId;

      myDlgId=AH_Dialog_GetDialogId(hmsg->dialog);
      assert(myDlgId);
      if (strcasecmp(myDlgId, p)!=0) {
        DBG_ERROR(AQHBCI_LOGDOMAIN, "Bad dialog id received from peer");
        GWEN_DB_SetIntValue(n, GWEN_DB_FLAGS_OVERWRITE_VARS,
                            "segment/error/code", 9210);
        GWEN_DB_SetCharValue(n, GWEN_DB_FLAGS_OVERWRITE_VARS,
                             "segment/error/text",
                             "Ungueltige Dialog Kennung");
        return GWEN_ERROR_BAD_DATA;
      } /* if bad dialog id */
    } /* if !initialtor */
  }
  n2=GWEN_DB_GetGroup(n,
                      GWEN_DB_FLAGS_DEFAULT |
                      GWEN_PATH_FLAGS_NAMEMUSTEXIST,
                      "msgref");
  if (n2) {
    /* we have a message reference, get it */
    const char *p;

    DBG_DEBUG(AQHBCI_LOGDOMAIN, "Found a message reference");
    p=GWEN_DB_GetCharValue(n2, "dialogid", 0, 0);
    if (!p) {
      DBG_WARN(AQHBCI_LOGDOMAIN,
	       "No reference dialog id in message, ignoring");
      /*return GWEN_ERROR_BAD_DATA;*/
    }
    else {
      if (strcasecmp(AH_Dialog_GetDialogId(hmsg->dialog), p)!=0) {
	/* some servers send error responses with invalid dialog id and
	 * message number 9999; we don't rely on the correct id anyway, so
	 * we might as well ignore that error here */
	DBG_WARN(AQHBCI_LOGDOMAIN, "Dialog id does not match current dialog id, ignoring");
	GWEN_Gui_ProgressLog(guiid,
			     GWEN_LoggerLevel_Warning,
			     I18N("Dialog id does not match, ignoring"));
	/*return GWEN_ERROR_BAD_DATA;*/
      }
      hmsg->refMsgNum=GWEN_DB_GetIntValue(n2, "msgnum", 0, 0);
      if (hmsg->refMsgNum!=expMsgNum) {
	DBG_WARN(AQHBCI_LOGDOMAIN, "Bad message reference number, ignoring");
	GWEN_Gui_ProgressLog(guiid,
			     GWEN_LoggerLevel_Warning,
			     I18N("Bad message reference number, ignoring"));
	hmsg->refMsgNum=expMsgNum;
      }
    }
  }
  else {
    DBG_DEBUG(AQHBCI_LOGDOMAIN, "No message reference found");
    GWEN_Gui_ProgressLog(guiid,
			 GWEN_LoggerLevel_Warning,
			 I18N("No message reference found, ignoring"));
  }

  /* find Crypt head */
  n=GWEN_DB_GetGroup(gr,
                     GWEN_DB_FLAGS_DEFAULT |
                     GWEN_PATH_FLAGS_NAMEMUSTEXIST,
                     "CryptHead");
  if (n) {
    if (GWEN_DB_GetIntValue(n, "segment/error/code", 0, 0)>=9000) {
      DBG_ERROR(AQHBCI_LOGDOMAIN, "Encryption error");
      return GWEN_ERROR_GENERIC;
    }
    rv=AH_Msg__Decrypt(hmsg, gr);
    if (rv) {
      DBG_INFO(AQHBCI_LOGDOMAIN, "here");
      return AB_ERROR_SECURITY;
    }
    /* unlink and delete crypthead */
    GWEN_DB_UnlinkGroup(n);
    GWEN_DB_Group_free(n);

    /* unlink and delete cryptdata */
    n=GWEN_DB_GetGroup(gr,
                       GWEN_DB_FLAGS_DEFAULT |
                       GWEN_PATH_FLAGS_NAMEMUSTEXIST,
                       "CryptData");
    if (n) {
      GWEN_DB_UnlinkGroup(n);
      GWEN_DB_Group_free(n);
    }

    /* parse decrypted message part */
    n=GWEN_DB_GetGroup(gr,
                       GWEN_DB_FLAGS_DEFAULT |
                       GWEN_PATH_FLAGS_NAMEMUSTEXIST,
                       "MsgTail");
    if (n) {
      /* temporarily unlink MsgTail, it will be appended after decoding
       * the crypted part, to keep the segment sequence correct */
      GWEN_DB_UnlinkGroup(n);
    }
    rv=AH_Msg_ReadMessage(hmsg, e, "SEG", hmsg->buffer, gr, flags);
    if (n)
      GWEN_DB_AddGroup(gr, n);
    if (rv) {
      DBG_INFO(AQHBCI_LOGDOMAIN, "here");
      return GWEN_ERROR_BAD_DATA;
    }
  } /* if crypthead */

  /* log decrypted message */
  AH_Msg_LogMessage(hmsg, hmsg->buffer, 1, 0);

  /* check segment sequence numbers */
  rv=AH_Msg_SequenceCheck(gr);
  if (rv) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "here");
    return AB_ERROR_SECURITY;
  }

  /* verify signatures */
  rv=AH_Msg__Verify(hmsg, gr);
  if (rv) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "here");
    return rv;
  }

  return 0;
}



/* --------------------------------------------------------------- FUNCTION */
void AH_Msg__Dump(const AH_MSG *hmsg,
                  unsigned int indent) {
  unsigned int i;
  GWEN_STRINGLISTENTRY *se;

  for (i=0; i<indent; i++) fprintf(stderr, " ");
  fprintf(stderr, "AH_Msg\n");
  for (i=0; i<indent; i++) fprintf(stderr, " ");
  fprintf(stderr, "==================================================\n");
  for (i=0; i<indent; i++) fprintf(stderr, " ");
  if (hmsg->origbuffer) {
    for (i=0; i<indent; i++) fprintf(stderr, " ");
    fprintf(stderr, "Original buffer      :\n");
    GWEN_Buffer_Dump(hmsg->origbuffer, indent+2);
  }
  else {
    for (i=0; i<indent; i++) fprintf(stderr, " ");
    fprintf(stderr, "Original buffer      : none\n");
  }
  if (hmsg->buffer) {
    for (i=0; i<indent; i++) fprintf(stderr, " ");
    fprintf(stderr, "Buffer:\n");
    GWEN_Buffer_Dump(hmsg->buffer, indent+2);
  }
  else {
    for (i=0; i<indent; i++) fprintf(stderr, " ");
    fprintf(stderr, "Buffer               : none\n");
  }

  for (i=0; i<indent; i++) fprintf(stderr, " ");
  if (hmsg->crypterId) {
    fprintf(stderr, "Crypter: %s\n", hmsg->crypterId);
  }
  else {
    fprintf(stderr, "Crypter: none\n");
  }
  for (i=0; i<indent; i++) fprintf(stderr, " ");
  fprintf(stderr, "Signers (%d):\n", GWEN_StringList_Count(hmsg->signerIdList));
  se=GWEN_StringList_FirstEntry(AH_Msg_GetSignerIdList(hmsg));
  while(se) {
    for (i=0; i<indent+2; i++) fprintf(stderr, " ");
    fprintf(stderr, "%s\n", GWEN_StringListEntry_Data(se));
    se=GWEN_StringListEntry_Next(se);
  } /* while */
  for (i=0; i<indent; i++) fprintf(stderr, " ");
  fprintf(stderr, "Nodes                : %d\n", hmsg->nodes);
  for (i=0; i<indent; i++) fprintf(stderr, " ");
  fprintf(stderr, "Msg number           : %d\n", hmsg->msgNum);
  for (i=0; i<indent; i++) fprintf(stderr, " ");
  fprintf(stderr, "Reference msg number : %d\n", hmsg->refMsgNum);
  for (i=0; i<indent; i++) fprintf(stderr, " ");
  fprintf(stderr, "First segment        : %d\n", hmsg->firstSegment);
  for (i=0; i<indent; i++) fprintf(stderr, " ");
  fprintf(stderr, "Last segment         : %d\n", hmsg->lastSegment);
  for (i=0; i<indent; i++) fprintf(stderr, " ");
  fprintf(stderr, "\n");
}



/* --------------------------------------------------------------- FUNCTION */
AH_DIALOG *AH_Msg_GetDialog(const AH_MSG *hmsg){
  assert(hmsg);
  return hmsg->dialog;
}



/* --------------------------------------------------------------- FUNCTION */
int AH_Msg_IsSignedBy(const AH_MSG *hmsg, const char *s){
  GWEN_STRINGLISTENTRY *se;

  se=GWEN_StringList_FirstEntry(hmsg->signerIdList);
  while(se) {
    if (strcasecmp(GWEN_StringListEntry_Data(se), s)==0)
      break;
    se=GWEN_StringListEntry_Next(se);
  }
  if (se) {
    DBG_DEBUG(AQHBCI_LOGDOMAIN, "Message signed by \"%s\"", s);
    return 1;
  }
  else {
    DBG_DEBUG(AQHBCI_LOGDOMAIN, "Customer \"%s\" did not sign the message", s);
    return 0;
  }
}



/* --------------------------------------------------------------- FUNCTION */
int AH_Msg_EnableInsert(AH_MSG *hmsg) {
  assert(hmsg);

  if (hmsg->nodes && !hmsg->enableInsert) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "There already are nodes in the buffer, "
             "cannot insert");
    return GWEN_ERROR_INVALID;
  }
  hmsg->enableInsert=1;
  return 0;
}



/* --------------------------------------------------------------- FUNCTION */
int AH_Msg_HasWarnings(const AH_MSG *hmsg) {
  assert(hmsg);
  return hmsg->hasWarnings;
}



/* --------------------------------------------------------------- FUNCTION */
void AH_Msg_SetHasWarnings(AH_MSG *hmsg, int i) {
  assert(hmsg);
  hmsg->hasWarnings=i;
}



/* --------------------------------------------------------------- FUNCTION */
int AH_Msg_HasErrors(const AH_MSG *hmsg) {
  assert(hmsg);
  return hmsg->hasErrors;
}



/* --------------------------------------------------------------- FUNCTION */
void AH_Msg_SetHasErrors(AH_MSG *hmsg, int i) {
  assert(hmsg);
  hmsg->hasErrors=i;
}



/* --------------------------------------------------------------- FUNCTION */
int AH_Msg_GetResultCode(const AH_MSG *hmsg){
  assert(hmsg);
  return hmsg->resultCode;
}



/* --------------------------------------------------------------- FUNCTION */
void AH_Msg_SetResultCode(AH_MSG *hmsg, int i){
  assert(hmsg);
  hmsg->resultCode=i;
}



/* --------------------------------------------------------------- FUNCTION */
const char *AH_Msg_GetResultText(const AH_MSG *hmsg){
  assert(hmsg);
  return hmsg->resultText;
}



/* --------------------------------------------------------------- FUNCTION */
void AH_Msg_SetResultText(AH_MSG *hmsg, const char *s){
  assert(hmsg);
  free(hmsg->resultText);
  if (s) hmsg->resultText=strdup(s);
  else hmsg->resultText=0;
}



/* --------------------------------------------------------------- FUNCTION */
const char *AH_Msg_GetResultParam(const AH_MSG *hmsg){
  assert(hmsg);
  return hmsg->resultParam;
}



/* --------------------------------------------------------------- FUNCTION */
void AH_Msg_SetResultParam(AH_MSG *hmsg, const char *s){
  assert(hmsg);
  free(hmsg->resultParam);
  if (s) hmsg->resultParam=strdup(s);
  else hmsg->resultParam=0;
}



/* --------------------------------------------------------------- FUNCTION */
unsigned int AH_Msg_GetHbciVersion(const AH_MSG *hmsg) {
  assert(hmsg);
  return hmsg->hbciVersion;
}



/* --------------------------------------------------------------- FUNCTION */
void AH_Msg_SetHbciVersion(AH_MSG *hmsg, unsigned int i) {
  assert(hmsg);
  hmsg->hbciVersion=i;
}



/* --------------------------------------------------------------- FUNCTION */
void AH_Msg_SetTan(AH_MSG *hmsg, const char *s){
  assert(hmsg);
  free(hmsg->usedTan);
  if (s) hmsg->usedTan=strdup(s);
  else hmsg->usedTan=0;
}



/* --------------------------------------------------------------- FUNCTION */
const char *AH_Msg_GetTan(const AH_MSG *hmsg){
  assert(hmsg);
  return hmsg->usedTan;
}



/* --------------------------------------------------------------- FUNCTION */
int AH_Msg_GetNeedTan(const AH_MSG *hmsg){
  assert(hmsg);
  return hmsg->needTan;
}



/* --------------------------------------------------------------- FUNCTION */
void AH_Msg_SetNeedTan(AH_MSG *hmsg, int i){
  assert(hmsg);
  hmsg->needTan=i;
}



/* --------------------------------------------------------------- FUNCTION */
int AH_Msg_NoSysId(const AH_MSG *hmsg){
  assert(hmsg);
  return hmsg->noSysId;
}



/* --------------------------------------------------------------- FUNCTION */
void AH_Msg_SetNoSysId(AH_MSG *hmsg, int i){
  assert(hmsg);
  hmsg->noSysId=i;
}






/* --------------------------------------------------------------- FUNCTION */
int AH_Msg__AnonHnsha(const char *psegment, unsigned int slen, GWEN_SYNCIO *sio){
  int plusCount=0;
  int lastWasEscape=0;
  int segDone=0;
  const char *p;
  unsigned int count;

  p=psegment;
  count=slen;
  while(*p && !segDone && count--) {
    int normalChar=1;
    int err;

    err=0;
    if (lastWasEscape) {
      lastWasEscape=0;
      normalChar=0;
    }
    else {
      if (*p=='?') {
	lastWasEscape=1;
      }
      else {
	if (*p=='\'')
	  segDone=1;
	else if (*p=='+')
	  plusCount++;
	lastWasEscape=0;
      }
    }
    if (plusCount>=3 && normalChar && *p!='+' && *p!='\'' && *p!=':')
      err=GWEN_SyncIo_WriteForced(sio, (const uint8_t*) "*", 1);
    else
      err=GWEN_SyncIo_WriteForced(sio, (const uint8_t*) p, 1);
    if (err<0) {
      DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", err);
      return err;
    }

    p++;
  } /* while */

  return 0;
}



/* --------------------------------------------------------------- FUNCTION */
int AH_Msg__AnonHkpae(const char *psegment, unsigned int slen, GWEN_SYNCIO *sio){
  int plusCount=0;
  int lastWasEscape=0;
  int segDone=0;
  const char *p;
  unsigned int count;

  p=psegment;
  count=slen;
  while(*p && !segDone && count--) {
    int normalChar=1;
    int err;

    err=0;
    if (lastWasEscape) {
      lastWasEscape=0;
      normalChar=0;
    }
    else {
      if (*p=='?') {
	lastWasEscape=1;
      }
      else {
	if (*p=='\'')
	  segDone=1;
	else if (*p=='+')
	  plusCount++;
	lastWasEscape=0;
      }
    }
    if (plusCount>=1 && normalChar && *p!='+' && *p!='\'' && *p!=':')
      err=GWEN_SyncIo_WriteForced(sio, (const uint8_t*) "*", 1);
    else
      err=GWEN_SyncIo_WriteForced(sio, (const uint8_t*) p, 1);
    if (err<0) {
      DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", err);
      return err;
    }

    p++;
  } /* while */

  return 0;
}



/* --------------------------------------------------------------- FUNCTION */
void AH_Msg_LogMessage(AH_MSG *msg,
                       GWEN_BUFFER *buf,
                       int rec,
                       int crypt) {
  GWEN_DB_NODE *db;
  AB_USER *u;
  AH_HBCI *h;
  GWEN_SYNCIO *sio;
  unsigned int bsize;
  const char *logFile;
  int vmajor, vminor, vpatchlevel, vbuild;
  char vbuf[32];
  int rv;

  assert(msg);
  assert(buf);

  logFile=AH_Dialog_GetLogFile(msg->dialog);
  if (!logFile) {
    DBG_ERROR(AQHBCI_LOGDOMAIN,
              "No log file, logging disabled for this dialog");
    return;
  }
  DBG_INFO(AQHBCI_LOGDOMAIN, "Logging to file [%s]", logFile);

  db=GWEN_DB_Group_new("header");
  u=AH_Dialog_GetDialogOwner(msg->dialog);
  h=AH_Dialog_GetHbci(msg->dialog);
  assert(h);

  GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS,
                       "mode",
                       AH_CryptMode_toString(AH_User_GetCryptMode(u)));
  GWEN_DB_SetIntValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS,
		      "rdhtype",
		      AH_User_GetRdhType(u));
  GWEN_DB_SetIntValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS,
		      "hbciVersion",
		      AH_User_GetHbciVersion(u));
  GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS,
		       "tokenType",
		       AH_User_GetTokenType(u));

  GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS,
                       "crypt",
                       crypt?"yes":"no");
  GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS,
                       "sender",
                       rec?"bank":"user");
  GWEN_Version(&vmajor, &vminor, &vpatchlevel, &vbuild);
  snprintf(vbuf, sizeof(vbuf)-1, "%d.%d.%d.%d",
	   vmajor, vminor, vpatchlevel, vbuild);
  vbuf[sizeof(vbuf)-1]=0;
  GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS,
		       "gwenhywfar", vbuf);

  GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS,
                       "aqhbci",
                       AQHBCI_VERSION_FULL_STRING);
  GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS,
                       "appname",
                       AH_HBCI_GetProductName(h));
  GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS,
                       "appversion",
		       AH_HBCI_GetProductVersion(h));
  GWEN_DB_SetIntValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS,
		      "size",
		      GWEN_Buffer_GetUsedBytes(buf));
  if (GWEN_Directory_GetPath(logFile,
			     GWEN_PATH_FLAGS_VARIABLE)) {
    DBG_ERROR(AQHBCI_LOGDOMAIN,
              "Path \"%s\" is not available, cannot log",
              logFile);
    GWEN_DB_Group_free(db);
    return;
  }

  sio=GWEN_SyncIo_File_new(logFile, GWEN_SyncIo_File_CreationMode_OpenAlways);
  GWEN_SyncIo_AddFlags(sio,
		       GWEN_SYNCIO_FILE_FLAGS_READ |
		       GWEN_SYNCIO_FILE_FLAGS_WRITE |
		       GWEN_SYNCIO_FILE_FLAGS_UREAD |
		       GWEN_SYNCIO_FILE_FLAGS_UWRITE |
		       GWEN_SYNCIO_FILE_FLAGS_APPEND);
  rv=GWEN_SyncIo_Connect(sio);
  if (rv<0) {
    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
    GWEN_SyncIo_free(sio);
    GWEN_DB_Group_free(db);
    return;
  }

  /* write header */
  rv=GWEN_DB_WriteToIo(db, sio,
		       GWEN_DB_FLAGS_WRITE_SUBGROUPS |
		       GWEN_DB_FLAGS_DETAILED_GROUPS |
		       GWEN_DB_FLAGS_USE_COLON|
		       GWEN_DB_FLAGS_OMIT_TYPES);
  if (rv<0) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
    GWEN_SyncIo_Disconnect(sio);
    GWEN_SyncIo_free(sio);
    GWEN_DB_Group_free(db);
    return;
  }

  /* append empty line to separate header from data */
  rv=GWEN_SyncIo_WriteForced(sio, (const uint8_t*) "\n", 1);
  if (rv<0) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
    GWEN_SyncIo_Disconnect(sio);
    GWEN_SyncIo_free(sio);
    GWEN_DB_Group_free(db);
    return;
  }

  /* write data */
  bsize=GWEN_Buffer_GetUsedBytes(buf);
  if (bsize && msg->usedPin) {
    char *p;
    unsigned int bleft;

    bleft=bsize;
    p=GWEN_Buffer_GetStart(buf);
    while(bleft) {
      char *segEnd;
      unsigned int slen;

      if (*p=='\'') {
	rv=GWEN_SyncIo_WriteForced(sio, (const uint8_t*) p, 1);
	if (rv<0) {
	  DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
	  GWEN_SyncIo_Disconnect(sio);
	  GWEN_SyncIo_free(sio);
	  GWEN_DB_Group_free(db);
	  return;
	}

	p++;
	bleft--;
      }
      else {
	segEnd=strchr(p, '\'');
	if (segEnd==NULL) {
	  /* no segment end found, write rest of the buffer */
	  rv=GWEN_SyncIo_WriteForced(sio, (const uint8_t*)p, bleft);
	  if (rv<0) {
	    DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
	    GWEN_SyncIo_Disconnect(sio);
	    GWEN_SyncIo_free(sio);
	    GWEN_DB_Group_free(db);
	    return;
	  }
	  break;
	}

	assert(segEnd);
	slen=segEnd-p+1;
	assert(slen);
  
	if (strncasecmp(p, "HNSHA:", 6)==0)
	  rv=AH_Msg__AnonHnsha(p, slen, sio);
	else if (strncasecmp(p, "HKPAE:", 6)==0 ||
		 strncasecmp(p, "DKPAE:", 6)==0)
	  rv=AH_Msg__AnonHkpae(p, slen, sio);
	/* add more segments with confidential data here */
	else {
	  unsigned int l;

	  l=slen;
	  rv=GWEN_SyncIo_WriteForced(sio, (const uint8_t*)p, l);
	}
	if (rv<0) {
	  DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
	  GWEN_SyncIo_Disconnect(sio);
	  GWEN_SyncIo_free(sio);
	  GWEN_DB_Group_free(db);
	  return;
	}

	bleft-=slen;
	p=segEnd+1;
      }
    } /* while bleft */
  }
  else {
    rv=GWEN_SyncIo_WriteForced(sio, (const uint8_t*)GWEN_Buffer_GetStart(buf), bsize);
    if (rv<0) {
      DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
      GWEN_SyncIo_Disconnect(sio);
      GWEN_SyncIo_free(sio);
      GWEN_DB_Group_free(db);
      return;
    }
  }

  /* add LF for better readability */
  rv=GWEN_SyncIo_WriteForced(sio, (const uint8_t*) "\n", 1);
  if (rv<0) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
    GWEN_SyncIo_Disconnect(sio);
    GWEN_SyncIo_free(sio);
    GWEN_DB_Group_free(db);
    return;
  }

  /* close layer */
  rv=GWEN_SyncIo_Disconnect(sio);
  if (rv<0) {
    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
    GWEN_SyncIo_free(sio);
    return;
  }

  GWEN_SyncIo_free(sio);

  GWEN_DB_Group_free(db);
  DBG_DEBUG(AQHBCI_LOGDOMAIN, "Message logged");
}



const char *AH_Msg_GetExpectedSigner(const AH_MSG *msg){
  assert(msg);
  return msg->expectedSigner;
}



void AH_Msg_SetExpectedSigner(AH_MSG *msg, const char *s){
  assert(msg);
  free(msg->expectedSigner);
  if (s) msg->expectedSigner=strdup(s);
  else msg->expectedSigner=0;
}



const char *AH_Msg_GetExpectedCrypter(const AH_MSG *msg){
  assert(msg);
  return msg->expectedCrypter;
}



void AH_Msg_SetExpectedCrypter(AH_MSG *msg, const char *s){
  assert(msg);
  free(msg->expectedCrypter);
  if (s) msg->expectedCrypter=strdup(s);
  else msg->expectedCrypter=0;
}



void AH_Msg_SetPin(AH_MSG *msg, const char *s){
  assert(msg);
  if (msg->usedPin) {
    memset(msg->usedPin, 0, strlen(msg->usedPin));
    free(msg->usedPin);
  }
  if (s) msg->usedPin=strdup(s);
  else msg->usedPin=0;
}



const char *AH_Msg_GetPin(const AH_MSG *msg){
  assert(msg);
  return msg->usedPin;
}



void AH_Msg_SetItanHashMode(AH_MSG *hmsg, int i) {
  assert(hmsg);
  hmsg->itanHashMode=i;
}



int AH_Msg_GetItanHashMode(const AH_MSG *hmsg) {
  assert(hmsg);
  return hmsg->itanHashMode;
}



void AH_Msg_SetItanHashBuffer(AH_MSG *hmsg, GWEN_BUFFER *hbuf) {
  assert(hmsg);
  if (hmsg->itanHashBuffer!=hbuf) {
    GWEN_Buffer_free(hmsg->itanHashBuffer);
    hmsg->itanHashBuffer=hbuf;
  }
}



GWEN_BUFFER *AH_Msg_GetItanHashBuffer(const AH_MSG *hmsg) {
  assert(hmsg);
  return hmsg->itanHashBuffer;
}



void AH_Msg_SetItanMethod(AH_MSG *hmsg, uint32_t i) {
  assert(hmsg);
  hmsg->itanMethod=i;
}



uint32_t AH_Msg_GetItanMethod(const AH_MSG *hmsg) {
  assert(hmsg);
  return hmsg->itanMethod;
}



const GWEN_STRINGLIST *AH_Msg_GetSignerIdList(const AH_MSG *hmsg) {
  assert(hmsg);
  return hmsg->signerIdList;
}



void AH_Msg_SetCrypterId(AH_MSG *hmsg, const char *s) {
  assert(hmsg);
  free(hmsg->crypterId);
  if (s)
    hmsg->crypterId=strdup(s);
  else
    hmsg->crypterId=NULL;
}



const char *AH_Msg_GetCrypterId(const AH_MSG *hmsg) {
  assert(hmsg);
  return hmsg->crypterId;
}





#include "msgcrypt_ddv.c"
#include "msgcrypt_rdh1.c"
#include "msgcrypt_rdh2.c"
#include "msgcrypt_rdh3.c"
#include "msgcrypt_rdh5.c"
#include "msgcrypt_rdh10.c"
#include "msgcrypt_rdh.c"
#include "msgcrypt_pintan.c"
#include "msgcrypt.inc"



