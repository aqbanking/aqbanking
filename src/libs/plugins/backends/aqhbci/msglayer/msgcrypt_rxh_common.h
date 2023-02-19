/***************************************************************************
    begin       : Tue Nov 25 2008
    copyright   : (C) 2020 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifndef AH_MSGCRYPT_RXH_COMMON_H
#define AH_MSGCRYPT_RXH_COMMON_H


#include "aqhbci/msglayer/hbci_l.h"
#include "aqhbci/msglayer/message_l.h"

#include <gwenhywfar/text.h>

#define AH_MSGRXH_MAXKEYBUF 4096


typedef enum {
  AH_Opmode_None=0,
  AH_Opmode_Cbc=2,
  AH_Opmode_Iso9796_1=16,
  AH_Opmode_Iso9796_2=17,
  AH_Opmode_Rsa_Pkcs1_v1_5=18,
  AH_Opmode_Rsa_Pss=19,
  AH_Opmode_Retail_MAC=999
} AH_OPMODE;

typedef enum {
  AH_HashAlg_None=0,
  AH_HashAlg_Sha1=1,
  AH_HashAlg_Sha256=3,
  AH_HashAlg_Sha256Sha256=6,
  AH_HashAlg_Ripmed160=999
} AH_HASH_ALG;

typedef enum {
  AH_SignAlg_DES=1,
  AH_SignAlg_RSA=10
} AH_SIGN_ALG;

typedef enum {
  AH_CryptAlg_2_Key_Triple_Des=13,
  AH_CryptAlg_AES256=14
} AH_CRYPT_ALG;

typedef enum {
  AH_UsageSign_None=0,
  AH_UsageSign_OwnerSigning=6
} AH_USAGE_SIGN;

typedef struct {
  AH_CRYPT_MODE protocol;
  uint8_t       protocolVersion;
  AH_SIGN_ALG   signAlgo;         /* Signaturalgorithmus, kodiert */
  AH_OPMODE     opmodSignS;       /* Operationsmodus bei Signatur (Signierschluessel) */
  AH_OPMODE     opmodSignD;       /* Operationsmodus bei Signatur (Signaturschluessel) */
  AH_USAGE_SIGN usageSign;        /* Verwendung des Signaturalgorithmus */
  AH_HASH_ALG   hashAlgS;         /* Hashalgorithmus, kodiert (Signierschluessel) */
  AH_HASH_ALG   hashAlgD;         /* Hashalgorithmus, kodiert (Signaturschluessel) */
  AH_CRYPT_ALG  cryptAlg;         /* Verschluesselungsalgorithmus, kodiert */
  AH_OPMODE     opmodCrypt;       /* Operationsmodus bei Verschluesselung */
} RXH_PARAMETER;


RXH_PARAMETER *AH_MsgRxh_GetParameters(AH_CRYPT_MODE cryptMode, int rxhVersion);



int AH_MsgRxh_PrepareCryptoSeg(AH_MSG *hmsg,
                               AB_USER *u,
                               RXH_PARAMETER *rxh_parameter,
                               int keyNum,
                               int keyVer,
                               const GWEN_CRYPT_TOKEN_KEYINFO *ki,
                               GWEN_DB_NODE *cfg,
                               int crypt,
                               int createCtrlRef);

int AH_Msg_SignRxh(AH_MSG *hmsg, GWEN_BUFFER *rawBuf, const char *signer);
int AH_Msg_EncryptRxh(AH_MSG *hmsg);
int AH_Msg_VerifyRxh(AH_MSG *hmsg, GWEN_DB_NODE *gr);






#endif



