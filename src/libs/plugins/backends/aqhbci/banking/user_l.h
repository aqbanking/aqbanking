/***************************************************************************
    begin       : Mon Mar 01 2004
    copyright   : (C) 2018 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifndef AH_USER_L_H
#define AH_USER_L_H


#include "aqhbci/banking/user.h"
#include "bpd_l.h"
#include "hbci_l.h"

#include <aqbanking/backendsupport/provider_be.h>

#include <gwenhywfar/msgengine.h>
#include <gwenhywfar/gui.h>


AB_USER *AH_User_new(AB_PROVIDER *pro);

AH_HBCI *AH_User_GetHbci(const AB_USER *u);
GWEN_MSGENGINE *AH_User_GetMsgEngine(const AB_USER *u);

int AH_User_GetBpdVersion(const AB_USER *u);
void AH_User_SetBpdVersion(AB_USER *u, int i);

AH_BPD *AH_User_GetBpd(const AB_USER *u);
void AH_User_SetBpd(AB_USER *u, AH_BPD *bpd);

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
GWEN_DB_NODE *AH_User_GetUpd(const AB_USER *u);

GWEN_DB_NODE *AH_User_GetUpdForAccount(const AB_USER *u, const AB_ACCOUNT *acc);
GWEN_DB_NODE *AH_User_GetUpdForAccountUniqueId(const AB_USER *u, uint32_t uid);


int AH_User_GetUpdVersion(const AB_USER *u);
void AH_User_SetUpdVersion(AB_USER *u, int i);


int AH_User_InputPin(AB_USER *u,
                     char *pwbuffer,
                     int minLen, int maxLen,
                     int flags);

int AH_User_InputTan(AB_USER *u,
                     char *pwbuffer,
                     int minLen,
                     int maxLen);

int AH_User_SetTanStatus(AB_USER *u,
                         const char *challenge,
                         const char *tan,
                         GWEN_GUI_PASSWORD_STATUS status);

int AH_User_SetPinStatus(AB_USER *u,
                         const char *pin,
                         GWEN_GUI_PASSWORD_STATUS status);


const GWEN_STRINGLIST *AH_User_GetSepaDescriptors(AB_USER *u);

int AH_User_VerifyInitialKey(GWEN_CRYPT_TOKEN *ct,
                             const GWEN_CRYPT_TOKEN_CONTEXT *ctx,
                             AB_USER *user,
                             GWEN_CRYPT_KEY *key,
                             uint16_t sentModl,
                             const char *keyName);


GWEN_DB_NODE *AH_User_GetBpdJobForParamNameAndVersion(const AB_USER *u, const char *paramName, int version);


#endif /* AH_USER_L_H */






