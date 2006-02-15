
#undef BUILDING_AQHBCI

#include <cbanking/cbanking.h>

#include "msglayer/medium_l.h"
#include <aqhbci/provider.h>
#include <aqhbci/hbci.h>

#include <aqbanking/banking.h>
#include <aqbanking/banking_be.h>

#include <gwenhywfar/args.h>
#include <gwenhywfar/buffer.h>
#include <gwenhywfar/db.h>
#include <gwenhywfar/debug.h>
#include <gwenhywfar/debug.h>

#include <unistd.h>



static int _getPin(AB_BANKING *ab,
                   GWEN_TYPE_UINT32 flags,
                   const char *token,
                   const char *title,
                   const char *text,
                   char *buffer,
                   int minLen,
                   int maxLen) {
  assert(maxLen>5);
  strcpy(buffer, "12345");
  return 0;
}



int check1() {
  AB_BANKING *ab;
  AB_PROVIDER *pro;
  AH_HBCI *hbci;
  int rv;
  AH_MEDIUM *medium=0;
  GWEN_CRYPTKEY *remotePrivCryptKey=0;
  GWEN_CRYPTKEY *remotePubCryptKey=0;
  GWEN_ERRORCODE err;
  GWEN_DB_NODE *dbKey;

  unlink("check1.medium");

  fprintf(stderr, "Check1:\n");
  ab=CBanking_new("hbci-check1", 0);
  CBanking_SetCharSet(ab, "ISO-8859-15");

  rv=AB_Banking_Init(ab);
  if (rv) {
    DBG_ERROR(0, "Error on init (%d)", rv);
    return 2;
  }

  AB_Banking_SetGetPinFn(ab, _getPin);

  pro=AB_Banking_GetProvider(ab, "aqhbci");
  assert(pro);
  hbci=AH_Provider_GetHbci(pro);
  assert(hbci);

  medium=AH_HBCI_MediumFactory(hbci, "ohbci", 0, "check1.medium");

  if (!medium) {
    DBG_ERROR(0, "Could not create medium object");
    return 3;
  }

  fprintf(stderr, "  Creating medium ...\n");
  rv=AH_Medium_Create(medium);
  if (rv) {
    DBG_ERROR(0, "Could not create medium (%d)", rv);
    return 3;
  }

  fprintf(stderr, "  Mounting medium ...\n");
  rv=AH_Medium_Mount(medium);
  if (rv) {
    DBG_ERROR(0, "Could not mount medium (%d)", rv);
    return 3;
  }

  fprintf(stderr, "  Selecting context ...\n");
  rv=AH_Medium_SelectContext(medium, 0);
  if (rv) {
    DBG_ERROR(0, "Could not select context (%d)", rv);
    return 3;
  }

  fprintf(stderr, "  Creating keys ...\n");
  rv=AH_Medium_CreateKeys(medium);
  if (rv) {
    DBG_ERROR(0, "Could not create keys (%d)", rv);
    return 3;
  }

  remotePrivCryptKey=GWEN_CryptKey_Factory("RSA");
  if (!remotePrivCryptKey) {
    DBG_ERROR(0, "Could not create key (%d)", rv);
    return 3;
  }

  fprintf(stderr, "  Generating remote crypt key ...\n");
  err=GWEN_CryptKey_Generate(remotePrivCryptKey, 768);
  if (!GWEN_Error_IsOk(err)) {
    DBG_ERROR_ERR(0, err);
    return 3;
  }

  fprintf(stderr, "  Extracting remote public key ...\n");
  dbKey=GWEN_DB_Group_new("key");
  err=GWEN_CryptKey_toDb(remotePrivCryptKey, dbKey, 1);
  if (!GWEN_Error_IsOk(err)) {
    DBG_ERROR_ERR(0, err);
    return 3;
  }
  remotePubCryptKey=GWEN_CryptKey_fromDb(dbKey);
  if (!remotePubCryptKey) {
    DBG_ERROR(0, "Could not create remote pub crypt key");
    return 3;
  }

  fprintf(stderr, "  Storing remote public key ...\n");
  rv=AH_Medium_SetPubCryptKey(medium, remotePubCryptKey);
  if (rv) {
    DBG_ERROR(0, "Could not store remote crypt key (%d)", rv);
    return 3;
  }


  fprintf(stderr, "  Unmounting medium ...\n");
  rv=AH_Medium_Unmount(medium, 1);
  if (rv) {
    DBG_ERROR(0, "Could not unmount medium (%d)", rv);
    return 3;
  }

  rv=AB_Banking_Fini(ab);
  if (rv) {
    DBG_ERROR(0, "Could not deinit banking (%d)", rv);
    return 3;
  }

  unlink("check1.medium");

  fprintf(stderr, "Check1: PASSED\n");
  return 0;
}



int check2() {
  AB_BANKING *ab;
  AB_PROVIDER *pro;
  AH_HBCI *hbci;
  int rv;
  AH_MEDIUM *medium=0;
  GWEN_CRYPTKEY *localCryptKey=0;
  GWEN_CRYPTKEY *msgKey=0;
  GWEN_ERRORCODE err;
  char keybuffer[16];
  unsigned int bsize;
  AH_MEDIUM_RESULT res;
  GWEN_BUFFER *plainKeyBuf;
  GWEN_BUFFER *encKeyBuf;
  GWEN_BUFFER *decKeyBuf;
  const char *p1, *p2;
  int i;

  fprintf(stderr, "Check2:\n");

  unlink("check2.medium");

  ab=CBanking_new("hbci-check1", 0);
  CBanking_SetCharSet(ab, "ISO-8859-15");

  rv=AB_Banking_Init(ab);
  if (rv) {
    DBG_ERROR(0, "Error on init (%d)", rv);
    return 2;
  }

  AB_Banking_SetGetPinFn(ab, _getPin);

  pro=AB_Banking_GetProvider(ab, "aqhbci");
  assert(pro);
  hbci=AH_Provider_GetHbci(pro);
  assert(hbci);

  medium=AH_HBCI_MediumFactory(hbci, "ohbci", 0, "check2.medium");

  if (!medium) {
    DBG_ERROR(0, "Could not create medium object");
    AB_Banking_Fini(ab);
    return 3;
  }

  fprintf(stderr, "  Creating medium ...\n");
  rv=AH_Medium_Create(medium);
  if (rv) {
    DBG_ERROR(0, "Could not create medium (%d)", rv);
    return 3;
  }

  fprintf(stderr, "  Mounting medium ...\n");
  rv=AH_Medium_Mount(medium);
  if (rv) {
    DBG_ERROR(0, "Could not mount medium (%d)", rv);
    return 3;
  }

  fprintf(stderr, "  Selecting context ...\n");
  rv=AH_Medium_SelectContext(medium, 0);
  if (rv) {
    DBG_ERROR(0, "Could not select context (%d)", rv);
    return 3;
  }

  fprintf(stderr, "  Creating keys ...\n");
  rv=AH_Medium_CreateKeys(medium);
  if (rv) {
    DBG_ERROR(0, "Could not create keys (%d)", rv);
    return 3;
  }

  localCryptKey=AH_Medium_GetLocalPubCryptKey(medium);
  if (!localCryptKey) {
    DBG_ERROR(0, "No local crypt key.");
    return 3;
  }

  fprintf(stderr, "  Storing remote public key ...\n");
  rv=AH_Medium_SetPubCryptKey(medium, localCryptKey);
  if (rv) {
    DBG_ERROR(0, "Could not store remote crypt key (%d)", rv);
    return 3;
  }

  fprintf(stderr, "  Creating DES key object ...\n");
  msgKey=GWEN_CryptKey_Factory("DES");
  if (!msgKey) {
    DBG_ERROR(0, "Could not create message key (%d)", rv);
    return 3;
  }

  fprintf(stderr, "  Generating DES message key ...\n");
  err=GWEN_CryptKey_Generate(msgKey, 16);
  if (!GWEN_Error_IsOk(err)) {
    DBG_ERROR_ERR(0, err);
    return 3;
  }

  fprintf(stderr, "  Getting data of message key ...\n");
  bsize=sizeof(keybuffer);
  err=GWEN_CryptKey_GetData(msgKey, keybuffer, &bsize);
  if (!GWEN_Error_IsOk(err)) {
    DBG_ERROR_ERR(0, err);
    return 3;
  }

  fprintf(stderr, "  Encrypting message key ...\n");
  plainKeyBuf=GWEN_Buffer_new(0, 256, 0, 1);
  GWEN_Buffer_AppendBytes(plainKeyBuf, keybuffer, bsize);
  encKeyBuf=GWEN_Buffer_new(0, 256, 0, 1);
  res=AH_Medium_EncryptKey(medium, plainKeyBuf, encKeyBuf);
  if (res!=AH_MediumResultOk) {
    DBG_ERROR(0, "Error %d", res);
    return 3;
  }

  fprintf(stderr, "  Decrypting message key ...\n");
  decKeyBuf=GWEN_Buffer_new(0, 256, 0, 1);
  res=AH_Medium_DecryptKey(medium, encKeyBuf, decKeyBuf);
  if (res!=AH_MediumResultOk) {
    DBG_ERROR(0, "Error %d", res);
    return 3;
  }

  fprintf(stderr, "  Comparing message key ...\n");
  p1=GWEN_Buffer_GetStart(plainKeyBuf);
  p2=GWEN_Buffer_GetStart(decKeyBuf);
  rv=0;
  for (i=0; i<GWEN_Buffer_GetUsedBytes(plainKeyBuf); i++) {
    if (p1[i]!=p2[i]) {
      fprintf(stderr, "Buffer1:\n");
      GWEN_Buffer_Dump(plainKeyBuf, stderr, 2);

      fprintf(stderr, "Buffer2:\n");
      GWEN_Buffer_Dump(decKeyBuf, stderr, 2);

      fprintf(stderr, "Differ at %d (%04x)\n", i, i);
      rv=-1;
      break;
    }
  }
  if (rv) {
    fprintf(stderr, "Data differs in content\n");
    return 3;
  }


  fprintf(stderr, "  Unmounting medium ...\n");
  rv=AH_Medium_Unmount(medium, 1);
  if (rv) {
    DBG_ERROR(0, "Could not unmount medium (%d)", rv);
    return 3;
  }

  rv=AB_Banking_Fini(ab);
  if (rv) {
    DBG_ERROR(0, "Could not deinit banking (%d)", rv);
    return 3;
  }

  unlink("check2.medium");

  fprintf(stderr, "Check2: PASSED\n");
  return 0;
}


int main(int argc, char **argv) {
  int rv;

  rv=check1();
  if (rv)
    return rv;
  rv=check2();
  if (rv)
    return rv;

  return 0;
}



