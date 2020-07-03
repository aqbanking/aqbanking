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


#include "s_hbci.h"

#include <gwenhywfar/debug.h>



/* ------------------------------------------------------------------------------------------------
 * forward declarations
 * ------------------------------------------------------------------------------------------------
 */


static AQFINTS_MESSAGE* GWENHYWFAR_CB _exchangeMessages(AQFINTS_SESSION *sess, AQFINTS_MESSAGE *messageOut);



/* ------------------------------------------------------------------------------------------------
 * implementations
 * ------------------------------------------------------------------------------------------------
 */





AQFINTS_SESSION *AQFINTS_SessionHbci_new(AQFINTS_PARSER *parser, AQFINTS_TRANSPORT *trans)
{
  AQFINTS_SESSION *sess;

  sess=AQFINTS_Session_new(parser, trans);
  assert(sess);

  AQFINTS_Session_SetExchangeMessagesFn(sess, _exchangeMessages);

  return sess;
}






AQFINTS_MESSAGE *_exchangeMessages(AQFINTS_SESSION *sess, AQFINTS_MESSAGE *messageOut)
{
  /* for now (later check for two-step tan) */
  return AQFINTS_Session_DirectlyExchangeMessages(sess, messageOut);
}





