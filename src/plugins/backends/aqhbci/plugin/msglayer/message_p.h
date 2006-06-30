/***************************************************************************
 $RCSfile$
                             -------------------
    cvs         : $Id$
    begin       : Mon Mar 01 2004
    copyright   : (C) 2004 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifndef AH_MESSAGE_P_H
#define AH_MESSAGE_P_H


#define AH_MSG_DEFAULTSIZE    512
#define AH_MSG_DEFAULTRESERVE 256
#define AH_MSG_DEFAULTSTEP    512

#include "message_l.h"
#include <gwenhywfar/keyspec.h>


struct AH_MSG {
  GWEN_LIST_ELEMENT(AH_MSG);

  AH_DIALOG *dialog;

  GWEN_BUFFER *buffer;
  GWEN_BUFFER *origbuffer;

  GWEN_KEYSPEC *crypter;
  GWEN_KEYSPEC_LIST *signers;
  unsigned int nSigners;

  char *expectedSigner;
  char *expectedCrypter;

  unsigned int hbciVersion;

  unsigned int nodes;
  unsigned int firstSegment;
  unsigned int lastSegment;

  unsigned int msgNum;
  unsigned int refMsgNum;

  int enableInsert;
  int hasWarnings;
  int hasErrors;

  int resultCode;
  char *resultText;
  char *resultParam;

  char *usedTan;
  int needTan;
  int noSysId;

  char *usedPin;

  int itanMethod;
  int itanHashMode;
  GWEN_BUFFER *itanHashBuffer;

  GWEN_DB_NODE *decodedMsg;
};


static void AH_Msg_SetPin(AH_MSG *hmsg, const char *s);

static int AH_Msg_AddMsgTail(AH_MSG *hmsg);
static int AH_Msg_AddMsgHead(AH_MSG *hmsg);

static int AH_Msg_PrepareCryptoSeg(AH_MSG *hmsg,
                                   AB_USER *u,
                                   GWEN_DB_NODE *cfg,
                                   const GWEN_KEYSPEC *ks,
                                   int crypt,
                                   int createCtrlRef);
static int AH_Msg_SignMsg(AH_MSG *hmsg,
                          GWEN_BUFFER *rawBuf,
                          const GWEN_KEYSPEC *ks);
static int AH_Msg_EncryptMsg(AH_MSG *hmsg);

static int AH_Msg_ReadSegment(AH_MSG *hmsg,
                              GWEN_MSGENGINE *e,
                              const char *gtype,
                              GWEN_BUFFER *mbuf,
                              GWEN_DB_NODE *gr,
                              unsigned int flags);
static int AH_Msg_ReadMessage(AH_MSG *hmsg,
                              GWEN_MSGENGINE *e,
                              const char *gtype,
                              GWEN_BUFFER *mbuf,
                              GWEN_DB_NODE *gr,
                              unsigned int flags);
static int AH_Msg_Decrypt(AH_MSG *hmsg, GWEN_DB_NODE *gr);
static int AH_Msg_SequenceCheck(GWEN_DB_NODE *gr);
static int AH_Msg_Verify(AH_MSG *hmsg,
                         GWEN_DB_NODE *gr,
                         unsigned int flags);

static int AH_Msg_PrepareCryptoSegDec(AH_MSG *hmsg,
                                      GWEN_DB_NODE *n,
                                      int crypt,
                                      GWEN_KEYSPEC **keySpec,
                                      int *signseq,
                                      const char **pSecurityId,
                                      int *lSecurityId,
                                      GWEN_BUFFER *msgkeybuf);


static GWEN_ERRORCODE AH_Msg__AnonHnsha(const char *psegment,
                                        unsigned int slen,
                                        GWEN_BUFFEREDIO *bio);
static GWEN_ERRORCODE AH_Msg__AnonHkpae(const char *psegment,
                                        unsigned int slen,
                                        GWEN_BUFFEREDIO *bio);


#endif /* AH_MESSAGE_P_H */



