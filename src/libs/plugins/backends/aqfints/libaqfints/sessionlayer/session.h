/***************************************************************************
 begin       : Thu Aug 01 2019
 copyright   : (C) 2019 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 * This file is part of the project "AqBanking".                           *
 * Please see toplevel file COPYING of that project for license details.   *
 ***************************************************************************/

#ifndef AQFINTS_SESSION_H
#define AQFINTS_SESSION_H


#include "libaqfints/aqfints.h"
#include "msglayer/message.h"
#include "msglayer/keydescr.h"
#include "parser/parser.h"
#include "transportlayer/transport.h"
#include "servicelayer/upd/userdata.h"
#include "servicelayer/bpd/bpd.h"
#include "sessionlayer/cryptparams.h"

#include <aqbanking/error.h>

#include <gwenhywfar/buffer.h>
#include <gwenhywfar/types.h>
#include <gwenhywfar/inherit.h>
#include <gwenhywfar/cryptkey.h>
#include <gwenhywfar/paddalgo.h>

#include <ctype.h>


typedef struct AQFINTS_SESSION AQFINTS_SESSION;
GWEN_INHERIT_FUNCTION_DEFS(AQFINTS_SESSION)


enum {
  AQFINTS_SESSION_CRYPTOP_UNKNOWN=-1,
  AQFINTS_SESSION_CRYPTOP_NONE,
  AQFINTS_SESSION_CRYPTOP_SIGN,
  AQFINTS_SESSION_CRYPTOP_ENCRYPT,
  AQFINTS_SESSION_CRYPTOP_AUTH,
};



/** @name Definitions for virtual functions
 *
 */
/*@{*/


/* definitions for virtual functions (post) */
typedef AQFINTS_MESSAGE* GWENHYWFAR_CB(*AQFINTS_SESSION_EXCHANGEMESSAGES_FN)(AQFINTS_SESSION *sess, AQFINTS_MESSAGE *messageOut);

typedef int GWENHYWFAR_CB(*AQFINTS_SESSION_FILLOUT_KEYDESCR_FN)(AQFINTS_SESSION *sess, AQFINTS_KEYDESCR *keyDescr, int mode);


typedef int GWENHYWFAR_CB(*AQFINTS_SESSION_DECRYPT_SKEY_FN)(AQFINTS_SESSION *sess,
                                                            const AQFINTS_KEYDESCR *keyDescr,
                                                            const AQFINTS_CRYPTPARAMS *cryptParams,
                                                            const uint8_t *pInData,
                                                            uint32_t inLen,
                                                            uint8_t *pOutData,
                                                            uint32_t *pOutLen);

typedef int GWENHYWFAR_CB(*AQFINTS_SESSION_ENCRYPT_SKEY_FN)(AQFINTS_SESSION *sess,
                                                            const AQFINTS_KEYDESCR *keyDescr,
                                                            const AQFINTS_CRYPTPARAMS *cryptParams,
                                                            const uint8_t *pInData,
                                                            uint32_t inLen,
                                                            uint8_t *pOutData,
                                                            uint32_t *pOutLen);

typedef int GWENHYWFAR_CB(*AQFINTS_SESSION_SIGN_FN)(AQFINTS_SESSION *sess,
                                                    const AQFINTS_KEYDESCR *keyDescr,
                                                    const AQFINTS_CRYPTPARAMS *cryptParams,
                                                    const uint8_t *pInData,
                                                    uint32_t inLen,
                                                    uint8_t *pSignatureData,
                                                    uint32_t *pSignatureLen);


/**
 * GWEN_ERROR_TRY_AGAIN: retry after handling the message
 */
typedef int GWENHYWFAR_CB(*AQFINTS_SESSION_VERIFY_FN)(AQFINTS_SESSION *sess,
                                                      const AQFINTS_KEYDESCR *keyDescr,
                                                      const AQFINTS_CRYPTPARAMS *cryptParams,
                                                      const uint8_t *pInData,
                                                      uint32_t inLen,
                                                      const uint8_t *pSignatureData,
                                                      uint32_t signatureLen,
                                                      uint32_t seqCounter);



typedef int GWENHYWFAR_CB(*AQFINTS_SESSION_VERIFYPIN_FN)(AQFINTS_SESSION *sess, const AQFINTS_KEYDESCR *keyDescr, const char *pin);

/*@}*/




/** @name Constructor, destructor
 *
 */
/*@{*/
AQFINTS_SESSION *AQFINTS_Session_new(AQFINTS_PARSER *parser, AQFINTS_TRANSPORT *trans);
void AQFINTS_Session_free(AQFINTS_SESSION *sess);

void AQFINTS_Session_Attach(AQFINTS_SESSION *sess);
/*@}*/




/** @name Variables to set before working with sessions
 *
 */
/*@{*/
int AQFINTS_Session_GetHbciVersion(const AQFINTS_SESSION *sess);
void AQFINTS_Session_SetHbciVersion(AQFINTS_SESSION *sess, int v);

int AQFINTS_Session_GetIsServer(const AQFINTS_SESSION *sess);
void AQFINTS_Session_SetIsServer(AQFINTS_SESSION *sess, int v);


/*@}*/




/** @name Variables set when parsing received messages
 *
 */
/*@{*/

const char *AQFINTS_Session_GetDialogId(const AQFINTS_SESSION *sess);
void AQFINTS_Session_SetDialogId(AQFINTS_SESSION *sess, const char *s);

/*@}*/




/** @name Internal functions to be used by the various session submodules
 *
 */
/*@{*/

AQFINTS_PARSER *AQFINTS_Session_GetParser(const AQFINTS_SESSION *sess);



int AQFINTS_Session_GetLastMessageNumSent(const AQFINTS_SESSION *sess);
void AQFINTS_Session_SetLastMessageNumSent(AQFINTS_SESSION *sess, int p_src);

int AQFINTS_Session_GetLastMessageNumReceived(const AQFINTS_SESSION *sess);
void AQFINTS_Session_SetLastMessageNumReceived(AQFINTS_SESSION *sess, int p_src);

/*@}*/



/** @name Sending, Receiving
 *
 */
/*@{*/

int AQFINTS_Session_Connect(AQFINTS_SESSION *sess);

int AQFINTS_Session_Disconnect(AQFINTS_SESSION *sess);

/**
 * @return message received (or NULL on error)
 * @param sess session pointer
 * @param messageOut Pointer to a message to be sent
 */
AQFINTS_MESSAGE *AQFINTS_Session_ExchangeMessages(AQFINTS_SESSION *sess, AQFINTS_MESSAGE *messageOut);

AQFINTS_MESSAGE *AQFINTS_Session_DirectlyExchangeMessages(AQFINTS_SESSION *sess, AQFINTS_MESSAGE *messageOut);

int AQFINTS_Session_SendMessage(AQFINTS_SESSION *sess, const char *ptrBuffer, int lenBuffer);

int AQFINTS_Session_ReceiveMessage(AQFINTS_SESSION *sess, GWEN_BUFFER *buffer);

/*@}*/



/** @name Cryptographic Functions
 *
 */
/*@{*/

int AQFINTS_Session_FilloutKeyname(AQFINTS_SESSION *sess, AQFINTS_KEYDESCR *keyDescr, int mode);

int AQFINTS_Session_DecryptSessionKey(AQFINTS_SESSION *sess,
                                      const AQFINTS_KEYDESCR *keyDescr,
                                      const AQFINTS_CRYPTPARAMS *cryptParams,
                                      const uint8_t *pInData,
                                      uint32_t inLen,
                                      uint8_t *pOutData,
                                      uint32_t *pOutLen);

int AQFINTS_Session_EncryptSessionKey(AQFINTS_SESSION *sess,
                                      const AQFINTS_KEYDESCR *keyDescr,
                                      const AQFINTS_CRYPTPARAMS *cryptParams,
                                      const uint8_t *pInData,
                                      uint32_t inLen,
                                      uint8_t *pOutData,
                                      uint32_t *pOutLen);

int AQFINTS_Session_VerifyPin(AQFINTS_SESSION *sess, const AQFINTS_KEYDESCR *keyDescr, const char *pin);


int AQFINTS_Session_Sign(AQFINTS_SESSION *sess,
                         const AQFINTS_KEYDESCR *keyDescr,
                         const AQFINTS_CRYPTPARAMS *cryptParams,
                         const uint8_t *pInData,
                         uint32_t inLen,
                         uint8_t *pSignatureData,
                         uint32_t *pSignatureLen);

/**
 *
 * @return GWEN_ERROR_TRY_AGAIN if the sign key is not yet available, 0 if okay, error code otherwise
 */
int AQFINTS_Session_Verify(AQFINTS_SESSION *sess,
                           const AQFINTS_KEYDESCR *keyDescr,
                           const AQFINTS_CRYPTPARAMS *cryptParams,
                           const uint8_t *pInData,
                           uint32_t inLen,
                           const uint8_t *pSignatureData,
                           uint32_t signatureLen,
                           uint32_t seqCounter);

/*@}*/



/** @name Setters for virtual functions
 *
 */
/*@{*/

AQFINTS_SESSION_EXCHANGEMESSAGES_FN AQFINTS_Session_SetExchangeMessagesFn(AQFINTS_SESSION *sess,
                                                                          AQFINTS_SESSION_EXCHANGEMESSAGES_FN fn);

AQFINTS_SESSION_DECRYPT_SKEY_FN AQFINTS_Session_SetDecryptSessionKeyFn(AQFINTS_SESSION *sess,
                                                                       AQFINTS_SESSION_DECRYPT_SKEY_FN fn);

AQFINTS_SESSION_ENCRYPT_SKEY_FN AQFINTS_Session_SetEncryptSessionKeyFn(AQFINTS_SESSION *sess,
                                                                       AQFINTS_SESSION_ENCRYPT_SKEY_FN fn);

AQFINTS_SESSION_VERIFYPIN_FN AQFINTS_Session_SetVerifyPinFn(AQFINTS_SESSION *sess,
                                                            AQFINTS_SESSION_VERIFYPIN_FN fn);


AQFINTS_SESSION_FILLOUT_KEYDESCR_FN AQFINTS_Session_SetFilloutKeynameFn(AQFINTS_SESSION *sess,
                                                                       AQFINTS_SESSION_FILLOUT_KEYDESCR_FN fn);

AQFINTS_SESSION_SIGN_FN AQFINTS_Session_SetSignFn(AQFINTS_SESSION *sess, AQFINTS_SESSION_SIGN_FN fn);

AQFINTS_SESSION_VERIFY_FN AQFINTS_Session_SetVerifyFn(AQFINTS_SESSION *sess, AQFINTS_SESSION_VERIFY_FN fn);


/*@}*/



/** @name Internal functions
 *
 */
/*@{*/


int AQFINTS_Session_WriteSegmentList(AQFINTS_SESSION *sess, AQFINTS_SEGMENT_LIST *segmentList);

int AQFINTS_Session_WriteSegment(AQFINTS_SESSION *sess, AQFINTS_SEGMENT *segment);


AQFINTS_BPD *AQFINTS_Session_ExtractBpdFromSegmentList(AQFINTS_SESSION *sess, AQFINTS_SEGMENT_LIST *segmentList);
AQFINTS_USERDATA_LIST *AQFINTS_Session_ExtractUpdFromSegmentList(AQFINTS_SESSION *sess,
                                                                 AQFINTS_SEGMENT_LIST *segmentList);

/**
 * Returns the number of TAN methods added.
 */
int AQFINTS_Session_SampleAllowedTanMethods(int *ptrIntArray, int sizeIntArray, AQFINTS_SEGMENT_LIST *segmentList);


int AQFINTS_Session_SampleDataToHash(AQFINTS_SEGMENT *segSigHead,
                                     AQFINTS_SEGMENT *segFirstToSign,
                                     AQFINTS_SEGMENT *segLastToSign,
                                     GWEN_BUFFER *destBuf);


void AQFINTS_Session_LogMessage(AQFINTS_SESSION *sess,
                                const uint8_t *ptrLogData,
                                uint32_t lenLogData,
                                int rec,
                                int crypt);


int AQFINTS_Session_GetAnonBpd(AQFINTS_SESSION *sess, const char *bankCode, AQFINTS_BPD **pBpd);


const char *AQFINTS_Session_GetLogFile(const AQFINTS_SESSION *sess);
void AQFINTS_Session_SetLogFile(AQFINTS_SESSION *sess, const char *s);

const char *AQFINTS_Session_GetAppRegKey(const AQFINTS_SESSION *sess);
void AQFINTS_Session_SetAppRegKey(AQFINTS_SESSION *sess, const char *s);

const char *AQFINTS_Session_GetAppVersion(const AQFINTS_SESSION *sess);
void AQFINTS_Session_SetAppVersion(AQFINTS_SESSION *sess, const char *s);



/*@}*/


#endif

