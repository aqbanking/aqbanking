
#undef BUILDING_AQHBCI

#include <aqhbci/provider.h>

#include <aqbanking/banking.h>
#include <aqbanking/banking_be.h>

#include <gwenhywfar/args.h>
#include <gwenhywfar/buffer.h>
#include <gwenhywfar/db.h>
#include <gwenhywfar/debug.h>
#include <gwenhywfar/debug.h>
#include <gwenhywfar/cgui.h>
#include <gwenhywfar/gui_be.h>

#include <unistd.h>



#if 0
static int _getPin(GWEN_GUI *gui,
		   uint32_t flags,
		   const char *token,
		   const char *title,
		   const char *text,
                   char *buffer,
                   int minLen,
		   int maxLen,
		   uint32_t guiid) {
  assert(maxLen>5);
  strcpy(buffer, "12345");
  return 0;
}


int check1() {
  AB_BANKING *ab;
  AB_PROVIDER *pro;
  int rv;
  AH_MEDIUM *medium=0;
  GWEN_CRYPTKEY *remotePrivCryptKey=0;
  GWEN_CRYPTKEY *remotePubCryptKey=0;
  int err;
  GWEN_DB_NODE *dbKey;
  GWEN_GUI *gui;

  gui=GWEN_Gui_CGui_new();
  GWEN_Gui_SetGetPasswordFn(gui, _getPin);

  unlink("check1.medium");

  fprintf(stderr, "Check1:\n");
  ab=AB_Banking_new("hbci-check1", 0, 0);

  rv=AB_Banking_Init(ab);
  if (rv) {
    DBG_ERROR(0, "Error on init (%d)", rv);
    return 2;
  }

  pro=AB_Banking_GetProvider(ab, "aqhbci");
  assert(pro);

  medium=AH_Provider_MediumFactory(pro, "ohbci", 0, "check1.medium");
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
  if (err) {
    DBG_ERROR_ERR(0, err);
    return 3;
  }

  fprintf(stderr, "  Extracting remote public key ...\n");
  dbKey=GWEN_DB_Group_new("key");
  err=GWEN_CryptKey_toDb(remotePrivCryptKey, dbKey, 1);
  if (err) {
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
  int rv;
  AH_MEDIUM *medium=0;
  GWEN_CRYPTKEY *localCryptKey=0;
  GWEN_CRYPTKEY *msgKey=0;
  int err;
  char keybuffer[16];
  unsigned int bsize;
  AH_MEDIUM_RESULT res;
  GWEN_BUFFER *plainKeyBuf;
  GWEN_BUFFER *encKeyBuf;
  GWEN_BUFFER *decKeyBuf;
  const char *p1, *p2;
  int i;
  GWEN_GUI *gui;

  fprintf(stderr, "Check2:\n");

  gui=GWEN_Gui_CGui_new();
  GWEN_Gui_SetGetPasswordFn(gui, _getPin);

  unlink("check2.medium");

  ab=AB_Banking_new("hbci-check1", 0, 0);

  rv=AB_Banking_Init(ab);
  if (rv) {
    DBG_ERROR(0, "Error on init (%d)", rv);
    return 2;
  }

  pro=AB_Banking_GetProvider(ab, "aqhbci");
  assert(pro);

  medium=AH_Provider_MediumFactory(pro, "ohbci", 0, "check2.medium");

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
  if (err) {
    DBG_ERROR_ERR(0, err);
    return 3;
  }

  fprintf(stderr, "  Getting data of message key ...\n");
  bsize=sizeof(keybuffer);
  err=GWEN_CryptKey_GetData(msgKey, keybuffer, &bsize);
  if (err) {
    DBG_ERROR_ERR(0, err);
    return 3;
  }

  fprintf(stderr, "  Encrypting message key ...\n");
  plainKeyBuf=GWEN_Buffer_new(0, 256, 0, 1);
  GWEN_Buffer_AppendBytes(plainKeyBuf, keybuffer, bsize);
  encKeyBuf=GWEN_Buffer_new(0, 256, 0, 1);
  res=AH_Medium_EncryptKey(medium, plainKeyBuf, encKeyBuf, 1);
  if (res!=AH_MediumResultOk) {
    DBG_ERROR(0, "Error %d", res);
    return 3;
  }

  fprintf(stderr, "  Decrypting message key ...\n");
  decKeyBuf=GWEN_Buffer_new(0, 256, 0, 1);
  res=AH_Medium_DecryptKey(medium, encKeyBuf, decKeyBuf, 1);
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


#endif

int main(int argc, char **argv) {
  fprintf(stderr, "Started...\n");
#if 0
  int rv;

  rv=check1();
  if (rv)
    return rv;
  rv=check2();
  if (rv)
    return rv;

#endif
  return 0;
}



