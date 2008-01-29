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

#ifndef AH_USER_H
#define AH_USER_H

#include <aqhbci/aqhbci.h>
#include <gwenhywfar/misc.h>


/** @defgroup G_AB_BE_AQHBCI_USER HBCI User Extensions
 * @ingroup G_AB_BE_AQHBCI
 * @short HBCI-specific user functions
 * @author Martin Preuss<martin@libchipcard.de>
 *
 */
/*@{*/

#include <aqbanking/user.h>

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
#define AH_USER_FLAGS_BANK_DOESNT_SIGN        0x00000001
/** bank uses correct signature sequence counters for its messages */
#define AH_USER_FLAGS_BANK_USES_SIGNSEQ       0x00000002
#define AH_USER_FLAGS_RESERVED1               0x00000004
#define AH_USER_FLAGS_RESERVED2               0x00000008
/** for pin/tan mode: keep HTTP connections alive (reuse for multiple
 * requests) */
#define AH_USER_FLAGS_KEEPALIVE               0x00000010
/** this flag is set automatically by AqHBCI upon BPD/UPD receiption. It
 * indicates that some jobs are supported even when there is no UPD job
 * description for it */
#define AH_USER_FLAGS_IGNORE_UPD              0x00000020

/** forces use of SSLv3 for Pin/TAN mode */
#define AH_USER_FLAGS_FORCE_SSL3              0x00000040

/** do not encode message in BASE64 (needed for APO bank) */
#define AH_USER_FLAGS_NO_BASE64               0x00000080
/*@}*/



/** @name HBCI TAN Methods
 *
 */
/*@{*/
/** single step PIN/TAN */
#define AH_USER_TANMETHOD_SINGLE_STEP        0x00000001
/** two step PIN/TAN, method 0 (990) */
#define AH_USER_TANMETHOD_TWO_STEP_0         0x00000002
/** two step PIN/TAN, method 1 (991) */
#define AH_USER_TANMETHOD_TWO_STEP_1         0x00000004
/** two step PIN/TAN, method 2 (992) */
#define AH_USER_TANMETHOD_TWO_STEP_2         0x00000008
/** two step PIN/TAN, method 3 (993) */
#define AH_USER_TANMETHOD_TWO_STEP_3         0x00000010
/** two step PIN/TAN, method 4 (994) */
#define AH_USER_TANMETHOD_TWO_STEP_4         0x00000020
/** two step PIN/TAN, method 5 (995) */
#define AH_USER_TANMETHOD_TWO_STEP_5         0x00000040
/** two step PIN/TAN, method 6 (996) */
#define AH_USER_TANMETHOD_TWO_STEP_6         0x00000060
/** two step PIN/TAN, method 7 (997) */
#define AH_USER_TANMETHOD_TWO_STEP_7         0x00000080
/** two step PIN/TAN, method 0 (900)  */
#define AH_USER_TANMETHOD_TWO_STEP_00        0x00000100
/** two step PIN/TAN, method 0 (901)  */
#define AH_USER_TANMETHOD_TWO_STEP_01        0x00000400
/** two step PIN/TAN, method 0 (902)  */
#define AH_USER_TANMETHOD_TWO_STEP_02        0x00000800
/** two step PIN/TAN, method 0 (903)  */
#define AH_USER_TANMETHOD_TWO_STEP_03        0x00001000
/** two step PIN/TAN, method 0 (904)  */
#define AH_USER_TANMETHOD_TWO_STEP_04        0x00002000
/** two step PIN/TAN, method 0 (905)  */
#define AH_USER_TANMETHOD_TWO_STEP_05        0x00004000
/** two step PIN/TAN, method 0 (906)  */
#define AH_USER_TANMETHOD_TWO_STEP_06        0x00008000
/** two step PIN/TAN, method 0 (907)  */
#define AH_USER_TANMETHOD_TWO_STEP_07        0x00010000
/*@}*/


/** @name Functions for Flags and Status
 *
 */
/*@{*/

AQHBCI_API
void AH_User_Flags_toDb(GWEN_DB_NODE *db, const char *name,
                        uint32_t flags);
AQHBCI_API
uint32_t AH_User_Flags_fromDb(GWEN_DB_NODE *db, const char *name);


AQHBCI_API
void AH_User_TanMethods_toDb(GWEN_DB_NODE *db, const char *name,
                             uint32_t m);

AQHBCI_API
uint32_t AH_User_TanMethods_fromDb(GWEN_DB_NODE *db,
                                           const char *name);


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
uint32_t AH_User_GetTanMethods(const AB_USER *u);

AQHBCI_API
void AH_User_SetTanMethods(AB_USER *u, uint32_t m);

AQHBCI_API
void AH_User_AddTanMethods(AB_USER *u, uint32_t m);

AQHBCI_API
void AH_User_SubTanMethods(AB_USER *u, uint32_t m);

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

int AH_User_GetRdhType(const AB_USER *u);
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


/*@}*/





/*@}*/ /* defgroup */

#ifdef __cplusplus
}
#endif

#endif /* AH_USER_H */






