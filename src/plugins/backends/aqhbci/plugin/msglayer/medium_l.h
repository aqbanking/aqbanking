/***************************************************************************
 $RCSfile$
                             -------------------
    cvs         : $Id$
    begin       : Mon Mar 01 2004
    copyright   : (C) 2004 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#ifndef AH_MEDIUM_L_H
#define AH_MEDIUM_L_H

#include <aqhbci/medium.h>
#include "hbci_l.h"

/**
 * @param mtn medium type name (like "RDHFile", "DDVCard" etc)
 * @param bankId bank code (German "Bankleitzahl")
 * @param userId id of this medium's owner
 */
AH_MEDIUM *AH_Medium_new(AH_HBCI *hbci,
                         const char *typeName,
                         const char *subTypeName,
                         const char *mediumName);

AH_HBCI *AH_Medium_GetHBCI(const AH_MEDIUM *m);
AB_BANKING *AH_Medium_GetBankingApi(const AH_MEDIUM *m);


/** @name Encryption And Decryption
 *
 */
/*@{*/
/**
 */

AH_MEDIUM_RESULT AH_Medium_Encrypt(AH_MEDIUM *m,
                                   GWEN_BUFFER *msgbuf,
                                   GWEN_BUFFER *encryptbuf,
                                   GWEN_BUFFER *msgKeyBuffer);

/**
 */
AH_MEDIUM_RESULT AH_Medium_Decrypt(AH_MEDIUM *m,
                                   GWEN_BUFFER *msgbuf,
                                   GWEN_BUFFER *decryptbuf,
                                   GWEN_BUFFER *msgKeyBuffer);
/*@}*/


/**
 * Stores all interesting data about this medium to the given DB (but not the
 * keys!) in order to make the medium restoreable via
 * @ref AH_Medium_FromDB.
 * This function stores the information it can access (like the keyspecs)
 * and then calls the implementation with a subgroup of the given DB.
 */
int AH_Medium_ToDB(const AH_MEDIUM *m, GWEN_DB_NODE *db);

/**
 * Restores the medium from the given DB.
 * This function restores the information it can access (like the keyspecs)
 * and then calls the implementation with a subgroup of the given DB.
 */
int AH_Medium_FromDB(AH_MEDIUM *m, GWEN_DB_NODE *db);

/**
 */
AH_MEDIUM_RESULT AH_Medium_Sign(AH_MEDIUM *m,
                                GWEN_BUFFER *msgbuf,
                                GWEN_BUFFER *signbuf);
/**
 */
int AH_Medium_GetNextSignSeq(AH_MEDIUM *m);


/**
 */
AQHBCI_API
AH_MEDIUM_RESULT AH_Medium_EncryptKey(AH_MEDIUM *m,
                                      GWEN_BUFFER *srckey,
                                      GWEN_BUFFER *encKey);

/**
 */
AQHBCI_API
AH_MEDIUM_RESULT AH_Medium_DecryptKey(AH_MEDIUM *m,
                                      GWEN_BUFFER *srckey,
                                      GWEN_BUFFER *decKey);

GWEN_BUFFER *AH_Medium_GenerateMsgKey(AH_MEDIUM *m);


/**
 * @param signseq Signature sequence counter received. If 0 then this
 * will not be verified, therefore the caller MUST make sure that a "0"
 * is never accepted from a signature inside a message !
 */
AH_MEDIUM_RESULT AH_Medium_Verify(AH_MEDIUM *m,
                                  GWEN_BUFFER *msgbuf,
                                  GWEN_BUFFER *signbuf,
                                  int signseq);


/** @name Helper Functions
 *
 */
/*@{*/

int AH_Medium_InputPin(AH_MEDIUM *m,
                       char *buffer,
                       int minLen,
                       int maxLen,
                       int flags);

int AH_Medium_SetPinStatus(AH_MEDIUM *m,
                           const char *pin,
                           AB_BANKING_PINSTATUS status);

int AH_Medium_InputTan(AH_MEDIUM *m,
                       char *buffer,
                       int minLen,
                       int maxLen);

int AH_Medium_SetTanStatus(AH_MEDIUM *m,
                           const char *tan,
                           AB_BANKING_TANSTATUS status);


/*@}*/



#ifdef __cplusplus
}
#endif

#endif /* AH_MEDIUM_L_H */


