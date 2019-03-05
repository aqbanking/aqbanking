/***************************************************************************
    begin       : Mon Mar 01 2004
    copyright   : (C) 2018 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifndef AQEBICS_CLIENT_PROVIDER_L_H
#define AQEBICS_CLIENT_PROVIDER_L_H

#include "aqebics/client/provider.h"
#include "aqebics/client/dialog_l.h"

#include <aqbanking/backendsupport/user.h>

#include <gwenhywfar/ct.h>
#include <gwenhywfar/cryptkey.h>

#include <libxml/tree.h>
#include <libxml/parser.h>
#include <libxml/xpath.h>


#define EBC_DEFAULT_CONNECT_TIMEOUT  30
#define EBC_DEFAULT_TRANSFER_TIMEOUT 60



int EBC_Provider_CreateKeys(AB_PROVIDER *pro,
                            AB_USER *u,
                            int cryptAndAuthKeySizeInBytes,
                            int signKeySizeInBytes,
                            int nounmount);

int EBC_Provider_CreateTempKey(AB_PROVIDER *pro,
                               AB_USER *u,
                               int signKeySizeInBytes,
                               int nounmount);

int EBC_Provider_GetIniLetterTxt(AB_PROVIDER *pro,
                                 AB_USER *u,
                                 int useBankKey,
                                 GWEN_BUFFER *lbuf,
                                 int nounmount);

int EBC_Provider_GetHiaLetterTxt(AB_PROVIDER *pro,
                                 AB_USER *u,
                                 int useBankKey,
                                 GWEN_BUFFER *lbuf,
                                 int nounmount);

int EBC_Provider_GetCert(AB_PROVIDER *pro, AB_USER *u);

int EBC_Provider_AddBankPubKeyDigests(AB_PROVIDER *pro, AB_USER *u, xmlNodePtr node);

int EBC_Provider_DecodeAndDecryptData(AB_PROVIDER *pro,
                                      AB_USER *u,
                                      GWEN_CRYPT_KEY *skey,
                                      const char *sEncryptedData,
                                      GWEN_BUFFER *targetBuffer);



int EBC_Provider_Send_HIA(AB_PROVIDER *pro, AB_USER *u, int doLock);
int EBC_Provider_Send_INI(AB_PROVIDER *pro, AB_USER *u, int doLock);
int EBC_Provider_Send_PUB(AB_PROVIDER *pro, AB_USER *u, const char *signVersion, int doLock);
int EBC_Provider_Send_HPB(AB_PROVIDER *pro, AB_USER *u, int doLock);
int EBC_Provider_Send_HPD(AB_PROVIDER *pro, AB_USER *u, int doLock);
int EBC_Provider_Send_HKD(AB_PROVIDER *pro, AB_USER *u, int doLock);
int EBC_Provider_Send_HTD(AB_PROVIDER *pro, AB_USER *u, int doLock);

int EBC_Provider_Download(AB_PROVIDER *pro, AB_USER *u,
                          const char *rtype,
                          GWEN_BUFFER *targetBuffer,
                          int withReceipt,
                          const GWEN_DATE *fromDate,
                          const GWEN_DATE *toDate,
                          int doLock);

int EBC_Provider_DownloadWithSession(AB_PROVIDER *pro,
                                     GWEN_HTTP_SESSION *sess,
                                     AB_USER *u,
                                     const char *rtype,
                                     GWEN_BUFFER *targetBuffer,
                                     int withReceipt,
                                     const GWEN_DATE *fromDate,
                                     const GWEN_DATE *toDate,
                                     int doLock);

int EBC_Provider_Upload(AB_PROVIDER *pro, AB_USER *u,
                        const char *rtype,
                        const uint8_t *pData,
                        uint32_t lData,
                        int doLock);

int EBC_Provider_DownloadIntoContext(AB_PROVIDER *pro,
                                     AB_USER *u,
                                     const char *rtype,
                                     int withReceipt,
                                     const GWEN_DATE *fromDate,
                                     const GWEN_DATE *toDate,
                                     const char *importerName,
                                     const char *profileName,
                                     AB_IMEXPORTER_CONTEXT *ctx,
                                     int doLock);

int EBC_Provider_DownloadIntoContextWithSession(AB_PROVIDER *pro,
                                                GWEN_HTTP_SESSION *sess,
                                                AB_USER *u,
                                                const char *rtype,
                                                int withReceipt,
                                                const GWEN_DATE *fromDate,
                                                const GWEN_DATE *toDate,
                                                const char *importerName,
                                                const char *profileName,
                                                AB_IMEXPORTER_CONTEXT *ctx,
                                                int doLock);


int EBC_Provider_GetConnectTimeout(const AB_PROVIDER *pro);
int EBC_Provider_GetTransferTimeout(const AB_PROVIDER *pro);





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
