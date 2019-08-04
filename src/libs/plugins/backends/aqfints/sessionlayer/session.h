/***************************************************************************
 begin       : Thu Aug 01 2019
 copyright   : (C) 2019 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 * This file is part of the project "AqBanking".                           *
 * Please see toplevel file COPYING of that project for license details.   *
 ***************************************************************************/

#ifndef AQFINTS_SESSION_H
#define AQFINTS_SESSION_H


#include "aqfints/aqfints.h"
#include "msglayer/message.h"
#include "msglayer/parser/parser.h"
#include "transportlayer/transport.h"
#include "servicelayer/upd/userdata.h"
#include "servicelayer/bpd/bpd.h"

#include <aqbanking/error.h>

#include <gwenhywfar/buffer.h>
#include <gwenhywfar/types.h>
#include <gwenhywfar/inherit.h>

#include <ctype.h>


typedef struct AQFINTS_SESSION AQFINTS_SESSION;
GWEN_INHERIT_FUNCTION_DEFS(AQFINTS_SESSION)



/* definitions for virtual functions (post) */
typedef int (*AQFINTS_SESSION_EXCHANGEMESSAGES_FN)(AQFINTS_SESSION *sess, AQFINTS_MESSAGE *messageOut,
                                                   AQFINTS_MESSAGE **pMessageIn);



AQFINTS_SESSION *AQFINTS_Session_new(AQFINTS_PARSER *parser, AQFINTS_TRANSPORT *trans);
void AQFINTS_Session_free(AQFINTS_SESSION *sess);

void AQFINTS_Session_Attach(AQFINTS_SESSION *sess);

int AQFINTS_Session_GetHbciVersion(const AQFINTS_SESSION *sess);
void AQFINTS_Session_SetHbciVersion(AQFINTS_SESSION *sess, int v);

const char *AQFINTS_Session_GetDialogId(const AQFINTS_SESSION *sess);
void AQFINTS_Session_SetDialogId(AQFINTS_SESSION *sess, const char *s);


int AQFINTS_Session_GetLastMessageNumSent(const AQFINTS_SESSION *sess);
void AQFINTS_Session_SetLastMessageNumSent(AQFINTS_SESSION *sess, int p_src);

int AQFINTS_Session_GetLastMessageNumReceived(const AQFINTS_SESSION *sess);
void AQFINTS_Session_SetLastMessageNumReceived(AQFINTS_SESSION *sess, int p_src);


AQFINTS_USERDATA_LIST *AQFINTS_Session_GetUserDataList(const AQFINTS_SESSION *sess);
void AQFINTS_Session_SetUserDataList(AQFINTS_SESSION *sess, AQFINTS_USERDATA_LIST *userDataList);

AQFINTS_BPD *AQFINTS_Session_GetBpd(const AQFINTS_SESSION *sess);
void AQFINTS_Session_SetBpd(AQFINTS_SESSION *sess, AQFINTS_BPD *bpd);


AQFINTS_PARSER *AQFINTS_Session_GetParser(const AQFINTS_SESSION *sess);


/* prototypes for virtual functions */
/**
 * @param messageOut Pointer to a message to be sent
 * @param pMessageIn Pointer to a pointer to receive a message
 */
int AQFINTS_Session_ExchangeMessages(AQFINTS_SESSION *sess,
                                     AQFINTS_MESSAGE *messageOut,
                                     AQFINTS_MESSAGE **pMessageIn);

/* setters for virtual functions */
AQFINTS_SESSION_EXCHANGEMESSAGES_FN AQFINTS_Session_SetExchangeMessagesFn(AQFINTS_SESSION *sess,
                                                                          AQFINTS_SESSION_EXCHANGEMESSAGES_FN fn);



int AQFINTS_Session_Connect(AQFINTS_SESSION *sess);

int AQFINTS_Session_Disconnect(AQFINTS_SESSION *sess);

int AQFINTS_Session_SendMessage(AQFINTS_SESSION *sess, const char *ptrBuffer, int lenBuffer);

int AQFINTS_Session_ReceiveMessage(AQFINTS_SESSION *sess, GWEN_BUFFER *buffer);



int AQFINTS_Session_WriteSegmentList(AQFINTS_SESSION *sess, AQFINTS_SEGMENT_LIST *segmentList,
                                     int firstSegNum, int refSegNum,
                                     GWEN_BUFFER *destBuffer);

int AQFINTS_Session_WriteSegment(AQFINTS_SESSION *sess, AQFINTS_SEGMENT *segment, GWEN_BUFFER *destBuffer);


int AQFINTS_Session_WrapMessageHeadAndTail(AQFINTS_SESSION *sess,
                                           int msgNum, int refMsgNum, int lastSegNum,
                                           GWEN_BUFFER *msgBuffer);

void AQFINTS_Session_ExtractBpdAndUpd(AQFINTS_SESSION *sess, AQFINTS_SEGMENT_LIST *segmentList);


int AQFINTS_Session_GetAnonBpd(AQFINTS_SESSION *sess, const char *bankCode);


#endif

