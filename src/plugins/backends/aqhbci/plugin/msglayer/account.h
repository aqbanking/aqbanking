/***************************************************************************
    begin       : Mon Mar 01 2004
    copyright   : (C) 2004 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifndef AH_ACCOUNT_H
#define AH_ACCOUNT_H

#include <aqhbci/aqhbci.h> /* for AQHBCI_API */
#include <aqbanking/provider.h>

/** @defgroup G_AB_BE_AQHBCI_Account HBCI Account Extensions
 * @ingroup G_AB_BE_AQHBCI
 * @short HBCI-specific user functions
 * @author Martin Preuss<martin@libchipcard.de>
 *
 */
/*@{*/

#ifdef __cplusplus
extern "C" {
#endif


/** @name Flags
 *
 */
/*@{*/
/** Prefer single transfers over multi transfers for this account */
#define AH_BANK_FLAGS_PREFER_SINGLE_TRANSFER  0x00000001
/** Prefer single debit notes over multi debit notes for this account */
#define AH_BANK_FLAGS_PREFER_SINGLE_DEBITNOTE 0x00000002

/* for this account there are KTV2 information availabel (e.g. account sub id) */
#define AH_BANK_FLAGS_KTV2                    0x00000004

/* this account can be used with SEPA jobs */
#define AH_BANK_FLAGS_SEPA                    0x00000008

/** Prefer single transfers over multi transfers for this account (SEPA) */
#define AH_BANK_FLAGS_SEPA_PREFER_SINGLE_TRANSFER  0x00000010
/** Prefer single debit notes over multi debit notes for this account (SEPA) */
#define AH_BANK_FLAGS_SEPA_PREFER_SINGLE_DEBITNOTE 0x00000020


#define AH_BANK_FLAGS_DEFAULT 0

/*@}*/


/** @name Flag Manipulation Functions
 *
 * See @ref AH_BANK_FLAGS_PREFER_SINGLE_TRANSFER and following.
 */
/*@{*/
AQHBCI_API
void AH_Account_Flags_toDb(GWEN_DB_NODE *db, const char *name,
                           uint32_t flags);

AQHBCI_API
uint32_t AH_Account_Flags_fromDb(GWEN_DB_NODE *db, const char *name);

AQHBCI_API
uint32_t AH_Account_GetFlags(const AB_ACCOUNT *a);

AQHBCI_API
void AH_Account_SetFlags(AB_ACCOUNT *a, uint32_t flags);

AQHBCI_API
void AH_Account_AddFlags(AB_ACCOUNT *a, uint32_t flags);

AQHBCI_API
void AH_Account_SubFlags(AB_ACCOUNT *a, uint32_t flags);

/*@}*/



#ifdef __cplusplus
}
#endif



/*@}*/ /* defgroup */


#endif /* AH_ACCOUNT_H */


