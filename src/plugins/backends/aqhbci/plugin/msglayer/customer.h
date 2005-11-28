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

#ifndef AH_CUSTOMER_H
#define AH_CUSTOMER_H

/** @defgroup MOD_CUSTOMER HBCI Customer
 *
 * @ingroup MOD_MSGLAYER
 *
 */
/*@{*/

#include <gwenhywfar/misc.h>
#include <gwenhywfar/misc2.h>
#include <aqhbci/objectref.h> /* for AQHBCI_API */

#ifdef __cplusplus
extern "C" {
#endif
typedef struct AH_CUSTOMER AH_CUSTOMER;
typedef struct AH_CUSTOMER_REF AH_CUSTOMER_REF;
GWEN_LIST2_FUNCTION_LIB_DEFS(AH_CUSTOMER, AH_Customer, AQHBCI_API);
#ifdef __cplusplus
}
#endif

#include <gwenhywfar/db.h>
#include <aqhbci/hbci.h>
#include <aqhbci/msgengine.h>
#include <aqhbci/bpd.h>


#ifdef __cplusplus
extern "C" {
#endif


AQHBCI_API
AH_CUSTOMER *AH_Customer_new(AH_USER *u, const char *customerId);
AQHBCI_API
void AH_Customer_Attach(AH_CUSTOMER *cu);
AQHBCI_API
void AH_Customer_free(AH_CUSTOMER *cu);

AQHBCI_API
int AH_Customer_GetBpdVersion(const AH_CUSTOMER *cu);
AQHBCI_API
void AH_Customer_SetBpdVersion(AH_CUSTOMER *cu, int i);

AQHBCI_API
AH_BPD *AH_Customer_GetBpd(const AH_CUSTOMER *cu);
AQHBCI_API
void AH_Customer_SetBpd(AH_CUSTOMER *cu, AH_BPD *bpd);


AQHBCI_API
GWEN_MSGENGINE *AH_Customer_GetMsgEngine(const AH_CUSTOMER *cu);
AQHBCI_API
void AH_Customer_SetMsgEngine(AH_CUSTOMER *cu, GWEN_MSGENGINE *e);

AQHBCI_API
int AH_Customer_GetHbciVersion(const AH_CUSTOMER *cu);
AQHBCI_API
void AH_Customer_SetHbciVersion(AH_CUSTOMER *cu, int i);

AQHBCI_API
AH_USER *AH_Customer_GetUser(const AH_CUSTOMER *cu);

AQHBCI_API
const char *AH_Customer_GetCustomerId(const AH_CUSTOMER *cu);

AQHBCI_API
const char *AH_Customer_GetFullName(const AH_CUSTOMER *cu);

AQHBCI_API
void AH_Customer_SetFullName(AH_CUSTOMER *cu,
                                  const char *s);

AQHBCI_API
int AH_Customer_GetUpdVersion(const AH_CUSTOMER *cu);
AQHBCI_API
void AH_Customer_SetUpdVersion(AH_CUSTOMER *cu, int i);

/**
 * Returns 0 if the bank doesn't sign messages, 1 otherwise.
 * This can be used in case the bank sends a sign key upon request but
 * never signs it's messages.
 */
AQHBCI_API
int AH_Customer_GetBankSigns(const AH_CUSTOMER *cu);

/**
 * Tell AqHBCI whether the bank this customer belongs to signs messages
 * for this customer.
 * @param b if 0 then the bank doesn't sign, otherwise it does
 */
AQHBCI_API
void AH_Customer_SetBankSigns(AH_CUSTOMER *cu, int b);


AQHBCI_API
int AH_Customer_GetBankUsesSignSeq(const AH_CUSTOMER *cu);

AQHBCI_API
void AH_Customer_SetBankUsesSignSeq(AH_CUSTOMER *cu, int b);


/**
 * Returns the major HTTP version to be used in PIN/TAN mode (defaults to 1).
 */
AQHBCI_API
int AH_Customer_GetHttpVMajor(const AH_CUSTOMER *cu);
AQHBCI_API
void AH_Customer_SetHttpVMajor(AH_CUSTOMER *cu, int i);

/**
 * Returns the minor HTTP version to be used in PIN/TAN mode (defaults to 1).
 */
AQHBCI_API
int AH_Customer_GetHttpVMinor(const AH_CUSTOMER *cu);
AQHBCI_API
void AH_Customer_SetHttpVMinor(AH_CUSTOMER *cu, int i);



AQHBCI_API
const char *AH_Customer_GetHttpUserAgent(const AH_CUSTOMER *cu);
AQHBCI_API
void AH_Customer_SetHttpUserAgent(AH_CUSTOMER *cu, const char *s);





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
GWEN_DB_NODE *AH_Customer_GetUpd(const AH_CUSTOMER *cu);
AQHBCI_API
void AH_Customer_SetUpd(AH_CUSTOMER *cu, GWEN_DB_NODE *n);


AQHBCI_API
int AH_Customer_IgnoreUPD(const AH_CUSTOMER *cu);
AQHBCI_API
void AH_Customer_SetIgnoreUPD(AH_CUSTOMER *cu, int i);

AQHBCI_API
int AH_Customer_GetPreferSingleTransfer(const AH_CUSTOMER *cu);
AQHBCI_API
void AH_Customer_SetPreferSingleTransfer(AH_CUSTOMER *cu, int i);


AQHBCI_API
int AH_Customer_GetPreferSingleDebitNote(const AH_CUSTOMER *cu);

AQHBCI_API
void AH_Customer_SetPreferSingleDebitNote(AH_CUSTOMER *cu, int i);


AQHBCI_API
int AH_Customer_GetKeepAlive(const AH_CUSTOMER *cu);

AQHBCI_API
void AH_Customer_SetKeepAlive(AH_CUSTOMER *cu, int i);


AQHBCI_API
const char *AH_Customer_GetSystemId(const AH_CUSTOMER *cu);
AQHBCI_API
void AH_Customer_SetSystemId(AH_CUSTOMER *cu, const char *s);


#ifdef __cplusplus
}
#endif

/*@}*/ /* defgroup */

#endif /* AH_CUSTOMER_H */


