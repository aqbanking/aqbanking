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


AQFINTS_SESSION *AQFINTS_Session_new(void);
void AQFINTS_Session_free(AQFINTS_SESSION *sess);

void AQFINTS_Session_Attach(AQFINTS_SESSION *sess);

int AQFINTS_Session_GetLastMessageNumSent(const AQFINTS_SESSION *sess);
void AQFINTS_Session_SetLastMessageNumSent(AQFINTS_SESSION *sess, int p_src);

int AQFINTS_Session_GetLastMessageNumReceived(const AQFINTS_SESSION *sess);
void AQFINTS_Session_SetLastMessageNumReceived(AQFINTS_SESSION *sess, int p_src);


AQFINTS_PARSER *AQFINTS_Session_GetParser(const AQFINTS_SESSION *sess);
void AQFINTS_Session_SetParser(AQFINTS_SESSION *sess, AQFINTS_PARSER *p_src);


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



#endif
