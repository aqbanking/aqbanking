/***************************************************************************
    begin       : Mon Mar 01 2004
    copyright   : (C) 2018 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifndef AH_MESSAGE_P_H
#define AH_MESSAGE_P_H


#define AH_MSG_DEFAULTSIZE    512
#define AH_MSG_DEFAULTRESERVE 256
#define AH_MSG_DEFAULTSTEP    512

#include "message_l.h"



struct AH_MSG {
  GWEN_LIST_ELEMENT(AH_MSG);

  AH_DIALOG *dialog;

  GWEN_BUFFER *buffer;
  GWEN_BUFFER *origbuffer;

  GWEN_STRINGLIST *signerIdList;
  char *crypterId;

  char *expectedSigner;
  char *expectedCrypter;

  unsigned int hbciVersion;
  int secProfile;
  int secClass;

  unsigned int nodes;
  unsigned int firstSegment;
  unsigned int lastSegment;

  unsigned int msgNum;
  unsigned int refMsgNum;

  int enableInsert;
  int hasWarnings;
  int hasErrors;

  int resultCode;
  char *resultText;
  char *resultParam;

  char *usedTan;
  int needTan;
  int noSysId;
  int signSeqOne; /* set signature sequence to 1 (used by AH_Msg_SignRxh) */

  char *usedPin;

  int itanMethod;
  int itanHashMode;
  GWEN_BUFFER *itanHashBuffer;

  GWEN_DB_NODE *decodedMsg;
};



#endif /* AH_MESSAGE_P_H */



