/***************************************************************************
    begin       : Mon Mar 01 2004
    copyright   : (C) 2004-2010 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifndef AQEBICS_CLIENT_PROVIDER_L_H
#define AQEBICS_CLIENT_PROVIDER_L_H

#include "provider.h"
#include "dialog_l.h"

#include <gwenhywfar/cryptkey.h>



int EBC_Provider_XchgIniRequest(AB_PROVIDER *pro,
                                GWEN_HTTP_SESSION *sess,
				AB_USER *u);

int EBC_Provider_XchgHiaRequest(AB_PROVIDER *pro,
				GWEN_HTTP_SESSION *sess,
				AB_USER *u);

int EBC_Provider_XchgPubRequest(AB_PROVIDER *pro,
				GWEN_HTTP_SESSION *sess,
				AB_USER *u,
				const char *signVersion);

int EBC_Provider_XchgHpbRequest(AB_PROVIDER *pro,
				GWEN_HTTP_SESSION *sess,
                                AB_USER *u);

int EBC_Provider_XchgHpdRequest(AB_PROVIDER *pro,
				GWEN_HTTP_SESSION *sess,
				AB_USER *u);

int EBC_Provider_XchgHkdRequest(AB_PROVIDER *pro,
				GWEN_HTTP_SESSION *sess,
				AB_USER *u);

int EBC_Provider_XchgHtdRequest(AB_PROVIDER *pro,
				GWEN_HTTP_SESSION *sess,
				AB_USER *u);

int EBC_Provider_XchgStaRequest(AB_PROVIDER *pro,
				GWEN_HTTP_SESSION *sess,
				const GWEN_TIME *fromTime,
				const GWEN_TIME *toTime,
				AB_IMEXPORTER_CONTEXT *ctx);


int EBC_Provider_SignMessage(AB_PROVIDER *pro,
			     EB_MSG *msg,
			     AB_USER *u,
			     xmlNodePtr node);

int EBC_Provider_ExtractSessionKey(AB_PROVIDER *pro,
				   AB_USER *u,
				   xmlNodePtr node,
				   GWEN_CRYPT_KEY **pKey);

int EBC_Provider_DecryptData(AB_PROVIDER *pro,
                             AB_USER *u,
			     GWEN_CRYPT_KEY *skey,
			     const uint8_t *p,
			     uint32_t len,
			     GWEN_BUFFER *msgBuffer);


int EBC_Provider_EncryptData(AB_PROVIDER *pro,
                             AB_USER *u,
			     GWEN_CRYPT_KEY *skey,
			     const uint8_t *pData,
			     uint32_t lData,
			     GWEN_BUFFER *sbuf);


int EBC_Provider_EncryptKey(AB_PROVIDER *pro,
			    AB_USER *u,
			    const GWEN_CRYPT_KEY *skey,
			    GWEN_BUFFER *sbuf);


int EBC_Provider_GenerateNonce(AB_PROVIDER *pro, GWEN_BUFFER *buf);
int EBC_Provider_GenerateTimeStamp(AB_PROVIDER *pro, AB_USER *u, GWEN_BUFFER *buf);
int EBC_Provider_Generate_OrderId(AB_PROVIDER *pro, GWEN_BUFFER *buf);

int EBC_Provider_MountToken(AB_PROVIDER *pro,
			    AB_USER *u,
			    GWEN_CRYPT_TOKEN **pCt,
			    const GWEN_CRYPT_TOKEN_CONTEXT **pCtx);

GWEN_LOGGER_LEVEL EBC_Provider_ResultCodeToLogLevel(AB_PROVIDER *pro, const char *s);

void EBC_Provider_LogRequestResults(AB_PROVIDER *pro,
				    EB_MSG *mRsp,
				    GWEN_BUFFER *logbuf);


int EBC_Provider_MkDownloadInitRequest(AB_PROVIDER *pro,
				       GWEN_HTTP_SESSION *sess,
				       AB_USER *u,
				       const char *requestType,
				       const GWEN_TIME *fromTime,
				       const GWEN_TIME *toTime,
				       EB_MSG **pMsg);

int EBC_Provider_MkDownloadTransferRequest(AB_PROVIDER *pro,
					   GWEN_HTTP_SESSION *sess,
					   AB_USER *u,
					   const char *transactionId,
                                           int segmentNumber,
					   EB_MSG **pMsg);

int EBC_Provider_MkDownloadReceiptRequest(AB_PROVIDER *pro,
					  GWEN_HTTP_SESSION *sess,
					  AB_USER *u,
					  const char *transactionId,
                                          int receiptCode,
					  EB_MSG **pMsg);

int EBC_Provider_XchgDownloadRequest(AB_PROVIDER *pro,
				     GWEN_HTTP_SESSION *sess,
				     AB_USER *u,
				     const char *requestType,
				     GWEN_BUFFER *targetBuffer,
				     int withReceipt,
				     const GWEN_TIME *fromTime,
				     const GWEN_TIME *toTime);


int EBC_Provider_MkUploadInitRequest(AB_PROVIDER *pro,
				     GWEN_HTTP_SESSION *sess,
				     AB_USER *u,
				     const char *requestType,
				     GWEN_CRYPT_KEY *skey,
				     const char *pEu,
				     uint32_t dlen,
				     EB_MSG **pMsg);

int EBC_Provider_MkUploadTransferRequest(AB_PROVIDER *pro,
					 GWEN_HTTP_SESSION *sess,
					 AB_USER *u,
					 const char *transactionId,
					 const char *pData,
					 uint32_t lData,
					 int segmentNumber,
					 int isLast,
					 EB_MSG **pMsg);

int EBC_Provider_XchgUploadRequest(AB_PROVIDER *pro,
				   GWEN_HTTP_SESSION *sess,
				   AB_USER *u,
				   const char *requestType,
				   const uint8_t *pData,
				   uint32_t lData);



int EBC_Provider_MkEuCryptZipDoc(AB_PROVIDER *pro,
				 AB_USER *u,
				 const char *requestType,
				 const uint8_t *pMsg,
				 uint32_t lMsg,
				 GWEN_CRYPT_KEY *skey,
				 GWEN_BUFFER *sbuf);

int EBC_Provider_FillDataEncryptionInfoNode(AB_PROVIDER *pro, AB_USER *u,
					    const GWEN_CRYPT_KEY *skey,
					    xmlNodePtr node);


int EBC_Provider_Sha256(const uint8_t *pData, uint32_t lData, GWEN_BUFFER *hbuf);


#endif
