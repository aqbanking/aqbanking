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


/** @defgroup MOD_USER HBCI User
 *
 * @ingroup MOD_MSGLAYER
 *
 */
/*@{*/

#include <aqhbci/medium.h>
#include <aqhbci/bpd.h>

#include <aqbanking/user.h>

#include <gwenhywfar/db.h>



#ifdef __cplusplus
extern "C" {
#endif

#define AH_USER_FLAGS_BANK_DOESNT_SIGN        0x00000001
#define AH_USER_FLAGS_BANK_USES_SIGNSEQ       0x00000002
#define AH_USER_FLAGS_PREFER_SINGLE_TRANSFER  0x00000004
#define AH_USER_FLAGS_PREFER_SINGLE_DEBITNOTE 0x00000008
#define AH_USER_FLAGS_KEEPALIVE               0x00000010
#define AH_USER_FLAGS_IGNORE_UPD              0x00000020

AQHBCI_API
void AH_User_Flags_toDb(GWEN_DB_NODE *db, const char *name,
                        GWEN_TYPE_UINT32 flags);
AQHBCI_API
GWEN_TYPE_UINT32 AH_User_Flags_fromDb(GWEN_DB_NODE *db, const char *name);


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


AQHBCI_API
AH_MEDIUM *AH_User_GetMedium(const AB_USER *u);
AQHBCI_API
void AH_User_SetMedium(AB_USER *u, AH_MEDIUM *m);


AQHBCI_API
const char *AH_User_GetPeerId(const AB_USER *u);
AQHBCI_API
void AH_User_SetPeerId(AB_USER *u, const char *s);


AQHBCI_API
int AH_User_GetContextIdx(const AB_USER *u);
AQHBCI_API
void AH_User_SetContextIdx(AB_USER *u, int idx);


AQHBCI_API
AH_CRYPT_MODE AH_User_GetCryptMode(const AB_USER *u);
AQHBCI_API
void AH_User_SetCryptMode(AB_USER *u, AH_CRYPT_MODE m);


AQHBCI_API
const AH_BPD_ADDR *AH_User_GetAddress(const AB_USER *u);
AQHBCI_API
void AH_User_SetAddress(AB_USER *u, const AH_BPD_ADDR *a);


AQHBCI_API
int AH_User_GetBpdVersion(const AB_USER *u);
AQHBCI_API
void AH_User_SetBpdVersion(AB_USER *u, int i);

AQHBCI_API
AH_BPD *AH_User_GetBpd(const AB_USER *u);
AQHBCI_API
void AH_User_SetBpd(AB_USER *u, AH_BPD *bpd);


AQHBCI_API
int AH_User_GetHbciVersion(const AB_USER *u);
AQHBCI_API
void AH_User_SetHbciVersion(AB_USER *u, int i);

AQHBCI_API
int AH_User_GetUpdVersion(const AB_USER *u);
AQHBCI_API
void AH_User_SetUpdVersion(AB_USER *u, int i);

/**
 * Returns 0 if the bank doesn't sign messages, 1 otherwise.
 * This can be used in case the bank sends a sign key upon request but
 * never signs it's messages.
 */
AQHBCI_API
GWEN_TYPE_UINT32 AH_User_GetFlags(const AB_USER *u);

AQHBCI_API
void AH_User_SetFlags(AB_USER *u, GWEN_TYPE_UINT32 flags);

AQHBCI_API
void AH_User_AddFlags(AB_USER *u, GWEN_TYPE_UINT32 flags);

AQHBCI_API
void AH_User_SubFlags(AB_USER *u, GWEN_TYPE_UINT32 flags);


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


/**
 * The upd (User Parameter Data) contains groups for every account
 * the customer has access to. The name of the group ressembles the
 * accountId. The structure is as follows (assuming an account id of
 * "123456" and a per day limit of 4000,- Euro for the job HKUEB which
 * is to be signed by at least one user):
 *
 * @code
 *
 * 11111 {
 *   updjob {
 *     char job="HKUEB"
 *     int  minsign="1"
 *     limit {
 *       char type="E"
 *       char value="4000,"
 *       char currency="EUR"
 *     } # limit
 *   } # updjob
 * } # 11111
 * @endcode
 *
 */
AQHBCI_API
GWEN_DB_NODE *AH_User_GetUpd(const AB_USER *u);
AQHBCI_API
void AH_User_SetUpd(AB_USER *u, GWEN_DB_NODE *n);

AQHBCI_API
const char *AH_User_GetSystemId(const AB_USER *u);
AQHBCI_API
void AH_User_SetSystemId(AB_USER *u, const char *s);



/*@}*/ /* defgroup */

#ifdef __cplusplus
}
#endif

#endif /* AH_USER_H */






