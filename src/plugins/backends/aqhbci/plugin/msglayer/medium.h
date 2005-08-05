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


#ifndef AH_MEDIUM_H
#define AH_MEDIUM_H

#include <gwenhywfar/misc.h>
#include <gwenhywfar/inherit.h>
#include <gwenhywfar/libloader.h>
#include <gwenhywfar/buffer.h>
#include <gwenhywfar/plugin.h>

#include <aqbanking/banking.h>
#include <aqhbci/objectref.h> /* for AQHBCI_API */

/**
 * If this flag is set then signing with an RSA key will not choose the
 * smaller signature according iso9796-appendix. The default is to allow it.
 */
#define AH_MEDIUM_FLAGS_DISABLE_SMALLER_SIGNATURE 0x00000001


#ifdef __cplusplus
extern "C" {
#endif
typedef struct AH_MEDIUM AH_MEDIUM;

typedef enum {
  AH_MediumTypeDDV=0,
  AH_MediumTypeRDH,
  AH_MediumTypePINTAN
} AH_MEDIUMTYPE;


typedef enum {
  AH_MediumResultOk=0,
  AH_MediumResultNoKey,
  AH_MediumResultBadKey,
  AH_MediumResultSignSeq,
  AH_MediumResultInvalidSignature,
  AH_MediumResultGenericError,
  AH_MediumResultNotSupported
} AH_MEDIUM_RESULT;


GWEN_LIST_FUNCTION_LIB_DEFS(AH_MEDIUM, AH_Medium, AQHBCI_API);

#ifdef __cplusplus
}
#endif


#include <gwenhywfar/buffer.h>
#include <gwenhywfar/crypt.h>
#include <aqhbci/hbci.h>

#include <aqhbci/mediumctx.h>

#ifdef __cplusplus
extern "C" {
#endif


/** @defgroup AQHBCI_MOD_HBCI_MEDIUM HBCI Medium
 * @ingroup AQHBCI_MOD_HBCI
 * @short Security Medium For HBCI
 * @author Martin Preuss<martin@libchipcard.de>
 *
 */
/*@{*/


/** @name Constructor And Destructor
 *
 */
/*@{*/
/**
 * @param mtn medium type name (like "RDHFile", "DDVCard" etc)
 * @param bankId bank code (German "Bankleitzahl")
 * @param userId id of this medium's owner
 */
AQHBCI_API
AH_MEDIUM *AH_Medium_new(AH_HBCI *hbci,
                         const char *typeName,
                         const char *subTypeName,
                         const char *mediumName);

/**
 * Destroys the medium if the internal usage counter reaches zero.
 * That counter is incremented by @ref AH_Medium_new and
 * @ref AH_Medium_Attach and decremented by this function.
 */
AQHBCI_API
void AH_Medium_free(AH_MEDIUM *m);

/**
 * Attaches to the medium. This means that the usage count of this medium
 * gets incremented so a following free will not really destroy it until
 * the usage counter reaches zero.
 * So after finishing using the medium attached to you should simply call
 * @ref AH_Medium_free and that function will take care of the usage
 * counter stuff.
 * Please note that @ref AH_Medium_new also increments the usage counter
 * so you do not have to call this function after creating the medium.
 */
AQHBCI_API
void AH_Medium_Attach(AH_MEDIUM *m);

/*@}*/


/** @name Informational Functions
 *
 */
/*@{*/

AQHBCI_API
AH_HBCI *AH_Medium_GetHBCI(const AH_MEDIUM *m);
AQHBCI_API
AB_BANKING *AH_Medium_GetBankingApi(const AH_MEDIUM *m);

AQHBCI_API
const char *AH_Medium_GetMediumTypeName(const AH_MEDIUM *m);

AQHBCI_API
const char *AH_Medium_GetMediumSubTypeName(const AH_MEDIUM *m);

AQHBCI_API
const char *AH_Medium_GetMediumName(const AH_MEDIUM *m);

AQHBCI_API
void AH_Medium_SetMediumName(AH_MEDIUM *m, const char *s);

/**
 * This string is presented to the user to allow him to identify the
 * medium an interactive function refers to (such as GetPin).
 * This string should be stored by the medium upon unmount and restored
 * when mounting the medium.
 */
AQHBCI_API
const char *AH_Medium_GetDescriptiveName(const AH_MEDIUM *m);
AQHBCI_API
void AH_Medium_SetDescriptiveName(AH_MEDIUM *m, const char *s);


AQHBCI_API
GWEN_TYPE_UINT32 AH_Medium_GetFlags(const AH_MEDIUM *m);
AQHBCI_API
void AH_Medium_SetFlags(AH_MEDIUM *m, GWEN_TYPE_UINT32 fl);
AQHBCI_API
void AH_Medium_AddFlags(AH_MEDIUM *m, GWEN_TYPE_UINT32 fl);
AQHBCI_API
void AH_Medium_SubFlags(AH_MEDIUM *m, GWEN_TYPE_UINT32 fl);

AQHBCI_API
int AH_Medium_IsMounted(AH_MEDIUM *m);

AQHBCI_API
int AH_Medium_GetTokenIdData(AH_MEDIUM *m, GWEN_BUFFER *buf);


/*@}*/


/** @name Context Selection, Creation and Removal
 *
 */
/*@{*/

AQHBCI_API
int AH_Medium_SelectContext(AH_MEDIUM *m, int idx);

AH_MEDIUM_CTX *AH_Medium_GetCurrentContext(AH_MEDIUM *m);

AQHBCI_API
int AH_Medium_ReadContext(AH_MEDIUM *m,
			  int idx,
			  int *country,
			  GWEN_BUFFER *bankId,
			  GWEN_BUFFER *userId,
			  GWEN_BUFFER *server,
			  int *port);

AQHBCI_API
int AH_Medium_WriteContext(AH_MEDIUM *m,
			   int idx,
			   int country,
			   const char *bankId,
			   const char *userId,
			   const char *server,
			   int port);


/*@}*/



/** @name Encryption And Decryption
 *
 */
/*@{*/
/**
 */
AQHBCI_API
AH_MEDIUM_RESULT AH_Medium_Encrypt(AH_MEDIUM *m,
                                   GWEN_BUFFER *msgbuf,
                                   GWEN_BUFFER *encryptbuf,
                                   GWEN_BUFFER *msgKeyBuffer);

/**
 */
AQHBCI_API
AH_MEDIUM_RESULT AH_Medium_Decrypt(AH_MEDIUM *m,
                                   GWEN_BUFFER *msgbuf,
                                   GWEN_BUFFER *decryptbuf,
                                   GWEN_BUFFER *msgKeyBuffer);
/*@}*/



/** @name Virtual Functions
 *
 * The functions in this group are wrappers which in most cases directly
 * call the implementations of the functions (exceptions:
 * @ref AH_Medium_Mount and @ref AH_Medium_Unmount).
 */
/*@{*/
/**
 * <p>
 * The implementation of this function should make the KeySpecs available
 * (using @ref AH_Medium_SetRemoteCryptKeySpec and others as
 * appropriate), at least id the medium is mounted for the first time in it's
 * lifetime. These keyspecs are then preserved upon
 * @ref AH_Medium_ToDB and @ref AH_Medium_fromDB.<br>
 * </p>
 * <p>
 * It generally is a good idea to update the keyspecs upon mount.
 * </p>
 *
 * <p>
 * Please note that this call only get's through to the virtual function of
 * inheriting classes if the mountCount is 0. Otherwise the mountCount will
 * simply be incremented without calling the implementation.
 * </p>
 *
 */
AQHBCI_API
int AH_Medium_Mount(AH_MEDIUM *m);

/**
 * Creates the medium. A KeyFile medium would prepare the file upon this.
 * Upon successful return this medium is mounted so you need to unmount it
 * in order to really write a keyfile.
 */
AQHBCI_API
int AH_Medium_Create(AH_MEDIUM *m);

/**
 *
 * Please note that this call only get's through to the virtual function of
 * inheriting classes if the mountCount is 1 or <i>force</i> is !=0.
 * Otherwise the mountCount will simply be decremented without calling the
 * implementation.
 */
AQHBCI_API
int AH_Medium_Unmount(AH_MEDIUM *m, int force);



/**
 * Makes the medium change the PIN.
 */
AQHBCI_API
int AH_Medium_ChangePin(AH_MEDIUM *m);


/**
 * Stores all interesting data about this medium to the given DB (but not the
 * keys!) in order to make the medium restoreable via
 * @ref AH_Medium_FromDB.
 * This function stores the information it can access (like the keyspecs)
 * and then calls the implementation with a subgroup of the given DB.
 */
AQHBCI_API
int AH_Medium_ToDB(const AH_MEDIUM *m, GWEN_DB_NODE *db);

/**
 * Restores the medium from the given DB.
 * This function restores the information it can access (like the keyspecs)
 * and then calls the implementation with a subgroup of the given DB.
 */
AQHBCI_API
int AH_Medium_FromDB(AH_MEDIUM *m, GWEN_DB_NODE *db);

/**
 */
AQHBCI_API
AH_MEDIUM_RESULT AH_Medium_Sign(AH_MEDIUM *m,
                                GWEN_BUFFER *msgbuf,
                                GWEN_BUFFER *signbuf);
/**
 */
AQHBCI_API
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

AQHBCI_API
GWEN_BUFFER *AH_Medium_GenerateMsgKey(AH_MEDIUM *m);


/**
 * @param signseq Signature sequence counter received. If 0 then this
 * will not be verified, therefore the caller MUST make sure that a "0"
 * is never accepted from a signature inside a message !
 */
AQHBCI_API
AH_MEDIUM_RESULT AH_Medium_Verify(AH_MEDIUM *m,
                                  GWEN_BUFFER *msgbuf,
                                  GWEN_BUFFER *signbuf,
                                  int signseq);

/*@}*/



/** @name Helper Functions
 *
 */
/*@{*/

AQHBCI_API
int AH_Medium_InputPin(AH_MEDIUM *m,
                       char *buffer,
                       int minLen,
                       int maxLen,
                       int flags);

AQHBCI_API
int AH_Medium_SetPinStatus(AH_MEDIUM *m,
                           const char *pin,
                           AB_BANKING_PINSTATUS status);

AQHBCI_API
int AH_Medium_InputTan(AH_MEDIUM *m,
                       char *buffer,
                       int minLen,
                       int maxLen);

AQHBCI_API
int AH_Medium_SetTanStatus(AH_MEDIUM *m,
                           const char *tan,
                           AB_BANKING_TANSTATUS status);


/*@}*/



/*@}*/ /* defgroup */


int AH_Medium_CreateKeys(AH_MEDIUM *m);

GWEN_CRYPTKEY *AH_Medium_GetLocalPubSignKey(AH_MEDIUM *m);
GWEN_CRYPTKEY *AH_Medium_GetLocalPubCryptKey(AH_MEDIUM *m);
GWEN_CRYPTKEY *AH_Medium_GetPubSignKey(AH_MEDIUM *m);
GWEN_CRYPTKEY *AH_Medium_GetPubCryptKey(AH_MEDIUM *m);
int AH_Medium_SetPubSignKey(AH_MEDIUM *m, const GWEN_CRYPTKEY *key);
int AH_Medium_SetPubCryptKey(AH_MEDIUM *m, const GWEN_CRYPTKEY *key);

int AH_Medium_ResetServerKeys(AH_MEDIUM *m);
int AH_Medium_ResetUserKeys(AH_MEDIUM *m);




#ifdef __cplusplus
}
#endif

#endif /* AH_MEDIUM_H */


