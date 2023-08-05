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
#include "aqhbci/banking/user_l.h"

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



int AH_Msg_VerifyPinTan(AH_MSG *hmsg, GWEN_DB_NODE *gr)
{
  AH_HBCI *h;
  GWEN_LIST *sigheads;
  GWEN_LIST *sigtails;
  GWEN_DB_NODE *n;
  int nonSigHeads;
  int nSigheads;
  unsigned int dataBegin;
//  char *dataStart;
//  unsigned int dataLength;
  unsigned int i;
  AB_USER *u;

  assert(hmsg);
  h=AH_Dialog_GetHbci(hmsg->dialog);
  assert(h);
  u=AH_Dialog_GetDialogOwner(hmsg->dialog);
  assert(u);

  /* let's go */
  sigheads=GWEN_List_new();

  /* enumerate signature heads */
  nonSigHeads=0;
  nSigheads=0;
  n=GWEN_DB_GetFirstGroup(gr);
  while (n) {
    if (strcasecmp(GWEN_DB_GroupName(n), "SigHead")==0) {
      /* found a signature head */
      if (nonSigHeads) {
        DBG_ERROR(AQHBCI_LOGDOMAIN, "Found some unsigned parts at the beginning");
        GWEN_List_free(sigheads);
        return GWEN_ERROR_BAD_DATA;
      }
      GWEN_List_PushBack(sigheads, n);
      nSigheads++;
    }
    else if (strcasecmp(GWEN_DB_GroupName(n), "MsgHead")!=0) {
      if (nSigheads)
        break;
      nonSigHeads++;
    }
    n=GWEN_DB_GetNextGroup(n);
  } /* while */

  if (!n) {
    if (nSigheads) {
      DBG_ERROR(AQHBCI_LOGDOMAIN,
                "Found Signature heads but no other segments");
      GWEN_List_free(sigheads);
      return GWEN_ERROR_BAD_DATA;
    }
    DBG_DEBUG(AQHBCI_LOGDOMAIN, "No signatures");
    GWEN_List_free(sigheads);
    return 0;
  }

  /* store begin of signed data */
  dataBegin=GWEN_DB_GetIntValue(n, "segment/pos", 0, 0);
  if (!dataBegin) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "No position specifications in segment");
    GWEN_List_free(sigheads);
    return GWEN_ERROR_BAD_DATA;
  }

  /* now get first signature tail */
  while (n) {
    if (strcasecmp(GWEN_DB_GroupName(n), "SigTail")==0) {
      unsigned int currpos;

      /* found a signature tail */
      currpos=GWEN_DB_GetIntValue(n, "segment/pos", 0, 0);
      if (!currpos || dataBegin>currpos) {
        DBG_ERROR(AQHBCI_LOGDOMAIN, "Bad position specification in Signature tail");
        GWEN_List_free(sigheads);
        return GWEN_ERROR_BAD_DATA;
      }
//      dataLength=currpos-dataBegin;
      break;
    }
    n=GWEN_DB_GetNextGroup(n);
  } /* while */

  if (!n) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "No signature tail found");
    GWEN_List_free(sigheads);
    return GWEN_ERROR_BAD_DATA;
  }

  sigtails=GWEN_List_new();
  while (n) {
    if (strcasecmp(GWEN_DB_GroupName(n), "SigTail")!=0)
      break;
    GWEN_List_PushBack(sigtails, n);
    n=GWEN_DB_GetNextGroup(n);
  } /* while */

  if (!n) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "Message tail expected");
    GWEN_List_free(sigheads);
    GWEN_List_free(sigtails);
    return GWEN_ERROR_BAD_DATA;
  }

  if (strcasecmp(GWEN_DB_GroupName(n), "MsgTail")!=0) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "Unexpected segment (msg tail expected)");
    GWEN_List_free(sigheads);
    GWEN_List_free(sigtails);
    return GWEN_ERROR_BAD_DATA;
  }

  n=GWEN_DB_GetNextGroup(n);
  if (n) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "Unexpected segment (end expected)");
    GWEN_List_free(sigheads);
    GWEN_List_free(sigtails);
    return GWEN_ERROR_BAD_DATA;
  }

  if (GWEN_List_GetSize(sigheads)!=
      GWEN_List_GetSize(sigtails)) {
    DBG_ERROR(AQHBCI_LOGDOMAIN,
              "Number of signature heads (%d) does not match "
              "number of signature tails (%d)",
              GWEN_List_GetSize(sigheads),
              GWEN_List_GetSize(sigtails));
    GWEN_List_free(sigheads);
    GWEN_List_free(sigtails);
    return GWEN_ERROR_BAD_DATA;
  }

  /* ok, now verify all signatures */
//  dataStart=GWEN_Buffer_GetStart(hmsg->buffer)+dataBegin;
  for (i=0; i< GWEN_List_GetSize(sigtails); i++) {
    GWEN_DB_NODE *sighead;
    GWEN_DB_NODE *sigtail;
    const char *signerId;

    /* get signature tail */
    sigtail=(GWEN_DB_NODE *)GWEN_List_GetBack(sigtails);

    /* get corresponding signature head */
    sighead=(GWEN_DB_NODE *)GWEN_List_GetFront(sigheads);

    if (!sighead || !sigtail) {
      DBG_ERROR(AQHBCI_LOGDOMAIN,
                "No signature head/tail left (internal error)");
      GWEN_List_free(sigheads);
      GWEN_List_free(sigtails);
      return GWEN_ERROR_INTERNAL;
    }

    GWEN_List_PopBack(sigtails);
    GWEN_List_PopFront(sigheads);

    signerId=GWEN_DB_GetCharValue(sighead, "key/userid", 0,
                                  I18N("unknown"));

    /* some checks */
    if (strcasecmp(GWEN_DB_GetCharValue(sighead, "ctrlref", 0, ""),
                   GWEN_DB_GetCharValue(sigtail, "ctrlref", 0, ""))!=0) {
      DBG_ERROR(AQHBCI_LOGDOMAIN, "Non-matching signature tail");
      GWEN_List_free(sigheads);
      GWEN_List_free(sigtails);
      return GWEN_ERROR_BAD_DATA;
    }

    /* verify signature */
    DBG_INFO(AQHBCI_LOGDOMAIN, "Message signed by \"%s\"", signerId);
    AH_Msg_AddSignerId(hmsg, signerId);

    DBG_DEBUG(AQHBCI_LOGDOMAIN, "Verification done");
  } /* for */

  GWEN_List_free(sigheads);
  GWEN_List_free(sigtails);
  return 0;
}

