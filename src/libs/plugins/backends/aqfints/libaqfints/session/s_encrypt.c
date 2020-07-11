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

#include "session/s_encrypt.h"
#include "session/pintan/s_encrypt_pintan.h"
#include "session/hbci/s_encrypt_hbci.h"
#include "parser/parser.h"

#include <gwenhywfar/misc.h>
#include <gwenhywfar/debug.h>
#include <gwenhywfar/text.h>



/* ------------------------------------------------------------------------------------------------
 * forward declarations
 * ------------------------------------------------------------------------------------------------
 */


/* ------------------------------------------------------------------------------------------------
 * implementations
 * ------------------------------------------------------------------------------------------------
 */



int AQFINTS_Session_EncryptMessage(AQFINTS_SESSION *sess, AQFINTS_MESSAGE *message)
{
  AQFINTS_KEYDESCR *keyDescr;
  const char *sSecProfileCode;

  keyDescr=AQFINTS_Message_GetCrypter(message);
  if (keyDescr==NULL) {
    DBG_ERROR(AQFINTS_LOGDOMAIN, "No crypter set");
    return GWEN_ERROR_GENERIC;
  }

  sSecProfileCode=AQFINTS_KeyDescr_GetSecurityProfileName(keyDescr);
  if (sSecProfileCode && *sSecProfileCode) {
    int rv;

    if (strcasecmp(sSecProfileCode, "PIN")==0)
      rv=AQFINTS_Session_EncryptMessagePinTan(sess, message);
    else if (strcasecmp(sSecProfileCode, "RDH")==0)
      rv=AQFINTS_Session_EncryptMessageHbci(sess, message);
    else if (strcasecmp(sSecProfileCode, "RAH")==0)
      rv=AQFINTS_Session_EncryptMessageHbci(sess, message);
    else {
      DBG_ERROR(AQFINTS_LOGDOMAIN, "Unhandled security profile \"%s\"", sSecProfileCode);
      return GWEN_ERROR_GENERIC;
    }

    if (rv<0) {
      DBG_ERROR(AQFINTS_LOGDOMAIN, "here (%d)", rv);
      return rv;
    }

    return 0;
  }
  else {
    DBG_ERROR(AQFINTS_LOGDOMAIN, "No security profile code set in session");
    return GWEN_ERROR_INVALID;
  }
}




