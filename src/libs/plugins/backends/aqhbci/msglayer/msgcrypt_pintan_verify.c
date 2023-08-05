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
                                GWEN_LIST *sigheads,
                                GWEN_LIST *sigtails,
                                unsigned int signedDataBeginPos,
                                unsigned int signedDataLength,
                                uint32_t gid);



/* ------------------------------------------------------------------------------------------------
 * implementations
 * ------------------------------------------------------------------------------------------------
 */



int AH_Msg_VerifyPinTan(AH_MSG *hmsg, GWEN_DB_NODE *gr)
{
  AH_HBCI *h;
  GWEN_LIST *sigheads;
  GWEN_LIST *sigtails;
  unsigned int signedDataBeginPos;
  unsigned int signedDataLength;
  AB_USER *u;
  uint32_t gid=0;
  int rv;

  assert(hmsg);
  h=AH_Dialog_GetHbci(hmsg->dialog);
  assert(h);
  u=AH_Dialog_GetDialogOwner(hmsg->dialog);
  assert(u);

  sigheads=GWEN_List_new();
  sigtails=GWEN_List_new();
  rv=AH_Msg_SampleSignHeadsAndTailsFromDecodedMsg(gr, sigheads, sigtails);
  if (rv<0) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
    GWEN_List_free(sigtails);
    GWEN_List_free(sigheads);
    return rv;
  }

  if (GWEN_List_GetSize(sigheads)==0) {
    DBG_DEBUG(AQHBCI_LOGDOMAIN, "No signatures");
    GWEN_List_free(sigtails);
    GWEN_List_free(sigheads);
    return 0;
  }

  rv=AH_Msg_GetStartPosOfSignedData(sigheads);
  if (rv<0) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
    GWEN_List_free(sigtails);
    GWEN_List_free(sigheads);
    return GWEN_ERROR_GENERIC;
  }
  signedDataBeginPos=(unsigned int) rv;

  rv=AH_Msg_GetFirstPosBehindSignedData(sigtails);
  if (rv<0 || ((unsigned int)rv)<signedDataBeginPos) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
    GWEN_List_free(sigtails);
    GWEN_List_free(sigheads);
    return GWEN_ERROR_GENERIC;
  }
  signedDataLength=((unsigned int) rv)-signedDataBeginPos;


  /* ok, now verify all signatures */
  rv=_verifyAllSignatures(hmsg, sigheads, sigtails, signedDataBeginPos, signedDataLength, gid);
  if (rv<0) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
    GWEN_List_free(sigheads);
    GWEN_List_free(sigtails);
    return rv;
  }

  GWEN_List_free(sigheads);
  GWEN_List_free(sigtails);
  return 0;
}



int _verifyAllSignatures(AH_MSG *hmsg, 
			 GWEN_LIST *sigheads,
			 GWEN_LIST *sigtails,
			 unsigned int signedDataBeginPos,
			 unsigned int signedDataLength,
			 uint32_t gid)
{
  /* not much to do here */
  return 0;
}




