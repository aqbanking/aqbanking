/***************************************************************************
 begin       : Sun Oct 27 2019
 copyright   : (C) 2019 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 * This file is part of the project "AqBanking".                           *
 * Please see toplevel file COPYING of that project for license details.   *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif


#include "s_pintan.h"

#include <gwenhywfar/debug.h>




static int GWENHYWFAR_CB _exchangeMessages(AQFINTS_SESSION *sess,
                                           AQFINTS_MESSAGE *messageOut,
                                           AQFINTS_MESSAGE **pMessageIn);





AQFINTS_SESSION *AQFINTS_SessionPinTan_new(AQFINTS_PARSER *parser, AQFINTS_TRANSPORT *trans)
{
  AQFINTS_SESSION *sess;

  sess=AQFINTS_Session_new(parser, trans);
  assert(sess);

  AQFINTS_Session_SetExchangeMessagesFn(sess, _exchangeMessages);

  return sess;
}






int _exchangeMessages(AQFINTS_SESSION *sess,
                      AQFINTS_MESSAGE *messageOut,
                      AQFINTS_MESSAGE **pMessageIn)
{
  return GWEN_ERROR_NOT_IMPLEMENTED;
}



