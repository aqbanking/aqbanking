/***************************************************************************
    begin       : Tue Nov 25 2008
    copyright   : (C) 2023 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifndef AH_MSGCRYPT_H
#define AH_MSGCRYPT_H


#include <gwenhywfar/db.h>
#include <gwenhywfar/list.h>


int AH_Msg_SampleSignHeadsAndTailsFromDecodedMsg(GWEN_DB_NODE *gr, GWEN_LIST *sigheads, GWEN_LIST *sigtails);
int AH_Msg_GetStartPosOfSignedData(const GWEN_LIST *sigheads);
int AH_Msg_GetFirstPosBehindSignedData(const GWEN_LIST *sigtails);



typedef int (*AH_MSG_VERIFY_SIGNATURES_FN)(AH_MSG *hmsg,
                                           GWEN_DB_NODE *dbParsedMsg,
                                           GWEN_LIST *sigheads,
                                           GWEN_LIST *sigtails,
                                           unsigned int signedDataBeginPos,
                                           unsigned int signedDataLength);
int AH_Msg_VerifyWithCallback(AH_MSG *hmsg, GWEN_DB_NODE *dbParsedMsg, AH_MSG_VERIFY_SIGNATURES_FN verifyCallback);


#endif
