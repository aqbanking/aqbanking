/***************************************************************************
    begin       : Mon Mar 01 2004
    copyright   : (C) 2019 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifndef AH_USER_H
#define AH_USER_H

#include "aqhbci/aqhbci.h"
#include "aqhbci/tan/tanmethod.h"

#include <gwenhywfar/misc.h>
#include <gwenhywfar/stringlist.h>
#include <gwenhywfar/cryptkeyrsa.h>


/** @defgroup G_AB_BE_AQHBCI_USER HBCI User Extensions
 * @ingroup G_AB_BE_AQHBCI
 * @short HBCI-specific user functions
 * @author Martin Preuss<martin@libchipcard.de>
 *
 */
/*@{*/

#include <aqbanking/backendsupport/user.h>

#include <gwenhywfar/db.h>
#include <gwenhywfar/url.h>



#ifdef __cplusplus
extern "C" {
#endif

/** @name HBCI User Flags
 *
 */
/*@{*/
/** bank doesn't sign its messages */
#define AH_USER_FLAGS_BANK_DOESNT_SIGN         0x00000001
/** bank uses correct signature sequence counters for its messages */
#define AH_USER_FLAGS_BANK_USES_SIGNSEQ        0x00000002
#define AH_USER_FLAGS_RESERVED1                0x00000004
#define AH_USER_FLAGS_RESERVED2                0x00000008

/** this flag is set automatically by AqHBCI upon BPD/UPD receiption. It
* indicates that some jobs are supported even when there is no UPD job
* description for it */
#define AH_USER_FLAGS_IGNORE_UPD               0x00000020

/** do not encode message in BASE64 (needed for APO bank) */
#define AH_USER_FLAGS_NO_BASE64                0x00000080

/** Normally the SWIFT parser removed double blanks, with this flag set it
 * no longer does (as requested by Andreas Filsinger).
 */
#define AH_USER_FLAGS_KEEP_MULTIPLE_BLANKS     0x00000100

/** Some TAN jobs have a field for the account to be used to charge
 * for SMS. This field is semi-optional (some banks need it, some need
 * you not to use it).
 * If this flag is set then the SMS account specification is omitted.
 */
#define AH_USER_FLAGS_TAN_OMIT_SMS_ACCOUNT      0x00000200

#define AH_USER_FLAGS_USE_STRICT_SEPA_CHARSET   0x00000800

/* ignore error "GNUTLS_E_PREMATURE_TERMINATION" */
/*#define AH_USER_FLAGS_TLS_IGN_PREMATURE_CLOSE   0x00001000 (ignored) */

/**
 * This flag is set when there is no public sign key of the bank
 * and the user has been informed about it.
 */
#define AH_USER_FLAGS_VERIFY_NO_BANKSIGNKEY     0x00002000

/**
 * This flag is set if HISPAS data in BPD job allows for national account spec (e.g. "BankCode",
 * "AccountId") in SEPA jobs (otherwise only IBAN and BIC are allowed)
 */
#define AH_USER_FLAGS_SEPA_ALLOWNATIONALACCSPEC 0x00004000
/*@}*/



/** @name Functions for Flags and Status
 *
 */
/*@{*/

AQHBCI_API
void AH_User_Flags_toDb(GWEN_DB_NODE *db, const char *name, uint32_t flags);
AQHBCI_API
uint32_t AH_User_Flags_fromDb(GWEN_DB_NODE *db, const char *name);


typedef enum {
  AH_UserStatusNew=0,
  AH_UserStatusEnabled,
  AH_UserStatusPending,
  AH_UserStatusDisabled,
  AH_UserStatusUnknown=999
} AH_USER_STATUS;
AQHBCI_API
const char *AH_User_Status_toString(AH_USER_STATUS st);
AQHBCI_API
AH_USER_STATUS AH_User_Status_fromString(const char *s);


AQHBCI_API
AH_USER_STATUS AH_User_GetStatus(const AB_USER *u);
AQHBCI_API
void AH_User_SetStatus(AB_USER *u, AH_USER_STATUS i);

/**
 * Returns 0 if the bank doesn't sign messages, 1 otherwise.
 * This can be used in case the bank sends a sign key upon request but
 * never signs it's messages.
 */
AQHBCI_API
uint32_t AH_User_GetFlags(const AB_USER *u);

AQHBCI_API
void AH_User_SetFlags(AB_USER *u, uint32_t flags);

AQHBCI_API
void AH_User_AddFlags(AB_USER *u, uint32_t flags);

AQHBCI_API
void AH_User_SubFlags(AB_USER *u, uint32_t flags);


/*@}*/


/** @name PIN/TAN Specific Functions
 *
 */
/*@{*/

AQHBCI_API
const int *AH_User_GetTanMethodList(const AB_USER *u);
AQHBCI_API
int AH_User_GetTanMethodCount(const AB_USER *u);
AQHBCI_API
int AH_User_HasTanMethod(const AB_USER *u, int method);
AQHBCI_API
int AH_User_HasTanMethodOtherThan(const AB_USER *u, int method);
AQHBCI_API
void AH_User_AddTanMethod(AB_USER *u, int method);
AQHBCI_API
void AH_User_ClearTanMethodList(AB_USER *u);

AQHBCI_API
int AH_User_GetSelectedTanMethod(const AB_USER *u);
AQHBCI_API
void AH_User_SetSelectedTanMethod(AB_USER *u, int i);


/**
 * Return selected TAN inpout method (see @ref AB_BANKING_TANMETHOD_TEXT and following).
 *
 * @return selected mechanism (0 if none selected)
 */
int AH_User_GetSelectedTanInputMechanism(const AB_USER *u);


void AH_User_SetSelectedTanInputMechanism(AB_USER *u, int i);


AQHBCI_API
const char *AH_User_GetHttpContentType(const AB_USER *u);
AQHBCI_API
void AH_User_SetHttpContentType(AB_USER *u, const char *s);

/*@}*/



AQHBCI_API
const char *AH_User_GetTokenType(const AB_USER *u);
AQHBCI_API
void AH_User_SetTokenType(AB_USER *u, const char *s);
AQHBCI_API
const char *AH_User_GetTokenName(const AB_USER *u);
AQHBCI_API
void AH_User_SetTokenName(AB_USER *u, const char *s);
AQHBCI_API
uint32_t AH_User_GetTokenContextId(const AB_USER *u);
AQHBCI_API
void AH_User_SetTokenContextId(AB_USER *u, uint32_t id);



/** @name Miscellanous Settings
 *
 */
/*@{*/
/**
 * Crypt mode (see @ref AH_CryptMode_Ddv and following).
 */
AQHBCI_API
AH_CRYPT_MODE AH_User_GetCryptMode(const AB_USER *u);
AQHBCI_API
void AH_User_SetCryptMode(AB_USER *u, AH_CRYPT_MODE m);

AQHBCI_API
int AH_User_GetRdhType(const AB_USER *u);

AQHBCI_API
void AH_User_SetRdhType(AB_USER *u, int i);

AQHBCI_API
const char *AH_User_GetPeerId(const AB_USER *u);
AQHBCI_API
void AH_User_SetPeerId(AB_USER *u, const char *s);

AQHBCI_API
const char *AH_User_GetSystemId(const AB_USER *u);
AQHBCI_API
void AH_User_SetSystemId(AB_USER *u, const char *s);


AQHBCI_API
const GWEN_URL *AH_User_GetServerUrl(const AB_USER *u);
AQHBCI_API
void AH_User_SetServerUrl(AB_USER *u, const GWEN_URL *url);


AQHBCI_API
int AH_User_GetHbciVersion(const AB_USER *u);
AQHBCI_API
void AH_User_SetHbciVersion(AB_USER *u, int i);


AQHBCI_API
const char *AH_User_GetSepaTransferProfile(const AB_USER *u);
AQHBCI_API
void AH_User_SetSepaTransferProfile(AB_USER *u, const char *profileName);
AQHBCI_API
const char *AH_User_GetSepaDebitNoteProfile(const AB_USER *u);
AQHBCI_API
void AH_User_SetSepaDebitNoteProfile(AB_USER *u, const char *profileName);
/*@}*/



/** @name Pin/Tan Settings
 *
 */
/*@{*/
/**
 * Returns the major HTTP version to be used in PIN/TAN mode (defaults to 1).
 */
AQHBCI_API
int AH_User_GetHttpVMajor(const AB_USER *u);
AQHBCI_API
void AH_User_SetHttpVMajor(AB_USER *u, int i);

/**
 * Returns the minor HTTP version to be used in PIN/TAN mode (defaults to 1).
 */
AQHBCI_API
int AH_User_GetHttpVMinor(const AB_USER *u);
AQHBCI_API
void AH_User_SetHttpVMinor(AB_USER *u, int i);



AQHBCI_API
const char *AH_User_GetHttpUserAgent(const AB_USER *u);
AQHBCI_API
void AH_User_SetHttpUserAgent(AB_USER *u, const char *s);


AQHBCI_API
const char *AH_User_GetTanMediumId(const AB_USER *u);

AQHBCI_API
void AH_User_SetTanMediumId(AB_USER *u, const char *s);


/*@}*/


/** @name Passwords/Pins/Tans
 *
 */
/*@{*/

AQHBCI_API
int AH_User_MkPasswdName(const AB_USER *u, GWEN_BUFFER *buf);

AQHBCI_API
int AH_User_MkPinName(const AB_USER *u, GWEN_BUFFER *buf);

AQHBCI_API
int AH_User_MkTanName(const AB_USER *u,
                      const char *challenge,
                      GWEN_BUFFER *buf);

/**
 * The list returned is only valid until the next call to this function!
 */
AQHBCI_API
const AH_TAN_METHOD_LIST *AH_User_GetTanMethodDescriptions(AB_USER *u);


/*@}*/


AQHBCI_API int AH_User_GetMaxTransfersPerJob(const AB_USER *u);
AQHBCI_API void AH_User_SetMaxTransfersPerJob(AB_USER *u, int i);
AQHBCI_API int AH_User_GetMaxDebitNotesPerJob(const AB_USER *u);
AQHBCI_API void AH_User_SetMaxDebitNotesPerJob(AB_USER *u, int i);

AQHBCI_API void AH_User_SetBankPubSignKey(AB_USER *u, GWEN_CRYPT_KEY *bankPubKey);
AQHBCI_API GWEN_CRYPT_KEY *AH_User_GetBankPubSignKey(const AB_USER *u);

AQHBCI_API void AH_User_SetBankPubCryptKey(AB_USER *u, GWEN_CRYPT_KEY *bankPubKey);
AQHBCI_API GWEN_CRYPT_KEY *AH_User_GetBankPubCryptKey(const AB_USER *u);

/*@}*/ /* defgroup */

#ifdef __cplusplus
}
#endif

#endif /* AH_USER_H */






