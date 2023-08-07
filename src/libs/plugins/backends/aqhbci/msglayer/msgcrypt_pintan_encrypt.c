/***************************************************************************
    begin       : Mon Mar 01 2004
    copyright   : (C) 2023 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif


#include "aqhbci/msglayer/msgcrypt_pintan_sign.h"
#include "aqhbci/msglayer/msgcrypt.h"
#include "aqhbci/banking/user_l.h"
#include "message_p.h"

#include "aqbanking/i18n_l.h"
#include "aqbanking/banking_be.h"



/* ------------------------------------------------------------------------------------------------
 * forward declarations
 * ------------------------------------------------------------------------------------------------
 */



/* ------------------------------------------------------------------------------------------------
 * implementations
 * ------------------------------------------------------------------------------------------------
 */


int AH_Msg_EncryptPinTan(AH_MSG *hmsg)
{
  AH_HBCI *h;
  GWEN_DB_NODE *cfg;
  GWEN_BUFFER *hbuf;
  int rv;
  const char *p;
  GWEN_MSGENGINE *e;
  AB_USER *u;

  assert(hmsg);
  h=AH_Dialog_GetHbci(hmsg->dialog);
  assert(h);
  e=AH_Dialog_GetMsgEngine(hmsg->dialog);
  assert(e);
  GWEN_MsgEngine_SetMode(e, "pintan");

  u=AH_Dialog_GetDialogOwner(hmsg->dialog);

  /* buffer for final message */
  hbuf=GWEN_Buffer_new(0, 256, 0, 1);

  /* create crypt head */
  cfg=GWEN_DB_Group_new("crypthead");
  GWEN_DB_SetIntValue(cfg, GWEN_DB_FLAGS_DEFAULT, "head/seq", 998);

  rv=AH_MsgPinTan_PrepareCryptoSeg(hmsg, u, cfg, 1, 0);
  if (rv) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
    GWEN_DB_Group_free(cfg);
    GWEN_Buffer_free(hbuf);
    return rv;
  }

  /* store system id */
  if (!hmsg->noSysId)
    p=AH_User_GetSystemId(u);
  else
    p=NULL;
  GWEN_DB_SetCharValue(cfg, GWEN_DB_FLAGS_DEFAULT, "SecDetails/SecId", p?p:"0");
  GWEN_DB_SetBinValue(cfg, GWEN_DB_FLAGS_DEFAULT, "CryptAlgo/MsgKey", "XXXXXXXX", 8);
  GWEN_DB_SetIntValue(cfg, GWEN_DB_FLAGS_DEFAULT, "CryptAlgo/keytype", 5);

  rv=AH_Msg_GenerateAndAddSegment(e, "CryptHead", cfg, hbuf);
  if (rv<0) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
    GWEN_DB_Group_free(cfg);
    GWEN_Buffer_free(hbuf);
    return GWEN_ERROR_INTERNAL;
  }
  GWEN_DB_Group_free(cfg);


  /* create cryptdata */
  cfg=GWEN_DB_Group_new("cryptdata");
  GWEN_DB_SetIntValue(cfg, GWEN_DB_FLAGS_DEFAULT, "head/seq", 999);
  GWEN_DB_SetBinValue(cfg, GWEN_DB_FLAGS_DEFAULT,
                      "cryptdata",
                      GWEN_Buffer_GetStart(hmsg->buffer),
                      GWEN_Buffer_GetUsedBytes(hmsg->buffer));

  rv=AH_Msg_GenerateAndAddSegment(e, "CryptData", cfg, hbuf);
  if (rv<0) {
    DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
    GWEN_Buffer_free(hbuf);
    GWEN_DB_Group_free(cfg);
    return GWEN_ERROR_INTERNAL;
  }
  GWEN_DB_Group_free(cfg);

  /* replace existing buffer by encrypted one */
  GWEN_Buffer_free(hmsg->buffer);
  hmsg->buffer=hbuf;

  return 0;
}




