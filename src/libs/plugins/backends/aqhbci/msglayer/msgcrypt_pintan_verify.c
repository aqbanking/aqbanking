/***************************************************************************
    begin       : Mon Mar 01 2004
    copyright   : (C) 2004 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif


#include "aqhbci/msglayer/msgcrypt_pintan_verify.h"
#include "aqhbci/msglayer/msgcrypt.h"
#include "aqhbci/banking/user_l.h"
#include "message_p.h"

#include "aqbanking/i18n_l.h"
#include "aqbanking/banking_be.h"



/* ------------------------------------------------------------------------------------------------
 * forward declarations
 * ------------------------------------------------------------------------------------------------
 */

static int _verifyAllSignatures(AH_MSG *hmsg,
                                GWEN_DB_NODE *dbParsedMsg,
                                GWEN_LIST *sigheads,
                                GWEN_LIST *sigtails,
                                unsigned int signedDataBeginPos,
                                unsigned int signedDataLength);



/* ------------------------------------------------------------------------------------------------
 * implementations
 * ------------------------------------------------------------------------------------------------
 */



int AH_Msg_VerifyPinTan(AH_MSG *hmsg, GWEN_DB_NODE *dbParsedMsg)
{
  int rv;

  rv=AH_Msg_VerifyWithCallback(hmsg, dbParsedMsg, _verifyAllSignatures);
  if (rv<0) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }
  return 0;
}



int _verifyAllSignatures(AH_MSG *hmsg,
                         GWEN_DB_NODE *dbParsedMsg,
                         GWEN_LIST *sigheads,
                         GWEN_LIST *sigtails,
                         unsigned int signedDataBeginPos,
                         unsigned int signedDataLength)
{
  /* in PINTAN mode there is no crypto stuff on the HBCI layer */
  return 0;
}




