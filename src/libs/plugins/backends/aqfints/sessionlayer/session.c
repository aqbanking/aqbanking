/***************************************************************************
 begin       : Thu Aug 01 2019
 copyright   : (C) 2019 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 * This file is part of the project "AqBanking".                           *
 * Please see toplevel file COPYING of that project for license details.   *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "./session_p.h"

#include <gwenhywfar/misc.h>
#include <gwenhywfar/debug.h>


GWEN_INHERIT_FUNCTIONS(AQFINTS_SESSION)



AQFINTS_SESSION *AQFINTS_Session_new(void)
{
  AQFINTS_SESSION *sess;

  GWEN_NEW_OBJECT(AQFINTS_SESSION, sess);
  sess->_refCount=1;
  GWEN_INHERIT_INIT(AQFINTS_SESSION, sess);
  sess->lastMessageNumSent=0;
  sess->lastMessageNumReceived=0;
  sess->parser=NULL;

  return sess;
}

void AQFINTS_Session_free(AQFINTS_SESSION *sess)
{
  if (sess) {
    assert(sess->_refCount);
    if (sess->_refCount==1) {
      GWEN_INHERIT_FINI(AQFINTS_SESSION, sess)
      sess->_refCount=0;
      GWEN_FREE_OBJECT(sess);
    }
    else
      sess->_refCount--;
  }
}



void AQFINTS_Session_Attach(AQFINTS_SESSION *sess)
{
  assert(sess);
  assert(sess->_refCount);
  sess->_refCount++;
}



int AQFINTS_Session_GetLastMessageNumSent(const AQFINTS_SESSION *sess)
{
  assert(sess);
  return sess->lastMessageNumSent;
}



int AQFINTS_Session_GetLastMessageNumReceived(const AQFINTS_SESSION *sess)
{
  assert(sess);
  return sess->lastMessageNumReceived;
}



AQFINTS_PARSER *AQFINTS_Session_GetParser(const AQFINTS_SESSION *sess)
{
  assert(sess);
  return sess->parser;
}



void AQFINTS_Session_SetLastMessageNumSent(AQFINTS_SESSION *sess, int p_src)
{
  assert(sess);
  sess->lastMessageNumSent=p_src;
}



void AQFINTS_Session_SetLastMessageNumReceived(AQFINTS_SESSION *sess, int p_src)
{
  assert(sess);
  sess->lastMessageNumReceived=p_src;
}



void AQFINTS_Session_SetParser(AQFINTS_SESSION *sess, AQFINTS_PARSER *p_src)
{
  assert(sess);
  sess->parser=p_src;
}



int AQFINTS_Session_ExchangeMessages(AQFINTS_SESSION *sess, AQFINTS_MESSAGE *messageOut,
                                     AQFINTS_MESSAGE **pMessageIn)
{
  assert(sess);
  if (sess->exchangeMessagesFn)
    return sess->exchangeMessagesFn(sess, messageOut, pMessageIn);
  else
    return GWEN_ERROR_NOT_IMPLEMENTED;
}



AQFINTS_SESSION_EXCHANGEMESSAGES_FN AQFINTS_Session_SetExchangeMessagesFn(AQFINTS_SESSION *sess,
                                                                          AQFINTS_SESSION_EXCHANGEMESSAGES_FN fn)
{
  AQFINTS_SESSION_EXCHANGEMESSAGES_FN oldFn;

  assert(sess);
  oldFn=sess->exchangeMessagesFn;
  sess->exchangeMessagesFn=fn;
  return oldFn;
}


